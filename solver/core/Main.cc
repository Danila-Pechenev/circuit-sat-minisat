/*****************************************************************************************[Main.cc]
Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
Copyright (c) 2007-2010, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include <errno.h>
#include <zlib.h>
#include <sstream>
#include <fstream>

#include "solver/utils/System.h"
#include "solver/utils/ParseUtils.h"
#include "solver/utils/Options.h"
#include "solver/core/Dimacs.h"
#include "solver/core/Solver.h"
#include "solver/core/Config.h"

#include "core/source/structures/parser.hpp"
#include "core/source/bench_to_cnf/bench_to_cnf.hpp"

using namespace Minisat;

//=================================================================================================

static Solver *solver;
// Terminate by notifying the solver and back out gracefully. This is mainly to have a test-case
// for this feature of the Solver as it may take longer than an immediate call to '_exit()'.
static void SIGINT_interrupt(int) { solver->interrupt(); }

// Note that '_exit()' rather than 'exit()' has to be used. The reason is that 'exit()' calls
// destructors and may cause deadlocks if a malloc/free function happens to be running (these
// functions are guarded by locks for multithreaded use).
static void SIGINT_exit(int)
{
    printf("\n");
    printf("*** INTERRUPTED ***\n");
    if (solver->verbosity > 0)
    {
        solver->printStats();
        printf("\n");
        printf("*** INTERRUPTED ***\n");
    }

    _exit(1);
}

//=================================================================================================
// Main:

int main(int argc, char **argv)
{
    try
    {
        setUsageHelp("USAGE: %s [options] <input-file> <result-output-file>\n\n  where input is BENCH file.\n");
        setX86FPUPrecision();

        // Extra options:
        //
        IntOption verb("MAIN", "verb", "Verbosity level (0=silent, 1=some, 2=more).", 0, IntRange(0, 2));
        IntOption cpu_lim("MAIN", "cpu-lim", "Limit on CPU time allowed in seconds.\n", 0, IntRange(0, INT32_MAX));
        IntOption mem_lim("MAIN", "mem-lim", "Limit on memory usage in megabytes.\n", 0, IntRange(0, INT32_MAX));
        BoolOption strictp("MAIN", "strict", "Validate DIMACS header during parsing.", false);
        BoolOption verify("MAIN", "verify", "Verify satisfying set if it is found.", false);
        BoolOption remove_cnf("MAIN", "remove-cnf", "Remove created cnf-file", true);

        parseOptions(argc, argv, true);

        Solver S;
        solver = &S;
        S.verbosity = verb;

        // Use signal handlers that forcibly quit until the solver will be able to respond to
        // interrupts:
        sigTerm(SIGINT_exit);

        // Try to set resource limits:
        if (cpu_lim != 0)
        {
            limitTime(cpu_lim);
        }

        if (mem_lim != 0)
        {
            limitMemory(mem_lim);
        }

        if (argc == 1)
        {
            printf("ERROR! Not enough arguments"), exit(1);
        }

        std::string bench_file = argv[1];
        std::ifstream file(bench_file);

        double initial_time = cpuTime();

        if (!file.is_open())
        {
            printf("ERROR! Could not open file: %s\n", argv[1]), exit(1);
        }

        auto parser = csat::BenchParser<csat::DAG>();
        parser.parseStream(file);
        file.clear();
        file.seekg(0);

        std::shared_ptr<csat::DAG> csat_instance = parser.instantiate();
        S.csat_instance = csat_instance;

        auto bench_to_cnf_parser = bench_to_cnf::BenchToCNFParser();
        bench_to_cnf_parser.parseStream(file);
        file.close();

        char* cnf_file_name = std::tmpnam(nullptr);  // Creating a unique filename that does not name a currently existing file
        std::ofstream cnf_file(cnf_file_name);
        bench_to_cnf_parser.writeCNFToStream(cnf_file);
        cnf_file.close();
        double transfer_to_cnf_time = cpuTime() - initial_time;

        if (remove_cnf)
        {
            remove(cnf_file_name);
        }

        initial_time = cpuTime();
        gzFile in = gzopen(cnf_file_name, "rb");
        if (in == NULL)
        {
            printf("ERROR! Could not open file: %s", cnf_file_name), exit(1);
        }

        if (S.verbosity > 0)
        {
            printf("============================[ Problem Statistics ]=============================\n");
            printf("|                                                                             |\n");
        }

        parse_DIMACS(in, S, (bool)strictp);
        gzclose(in);

        FILE *res = (argc >= 3) ? fopen(argv[2], "wb") : NULL; // file for writing satisfying set

        if (S.verbosity > 0)
        {
            printf("|  Number of variables:  %12d                                         |\n", S.nVars());
            printf("|  Number of clauses:    %12d                                         |\n", S.nClauses());
        }

        double parsed_time = cpuTime();
        if (S.verbosity > 0)
        {
            printf("|  Transfer to CNF time: %12.2f s                                       |\n", transfer_to_cnf_time);
            printf("|  Parse time:           %12.2f s                                       |\n", parsed_time - initial_time);
            printf("|                                                                             |\n");
        }

        // Change to signal-handlers that will only notify the solver and allow it to terminate
        // voluntarily:
        sigTerm(SIGINT_interrupt);

        if (!S.simplify())
        {
            if (res != NULL)
            {
                fprintf(res, "UNSAT\n"), fclose(res);
            }

            if (S.verbosity > 0)
            {
                printf("===============================================================================\n");
                printf("Solved by unit propagation\n");
                S.printStats();
                printf("\n");
            }

            printf("UNSATISFIABLE\n");
            exit(20);
        }

        vec<Lit> dummy;
        lbool ret = S.solveLimited(dummy);
        if (S.verbosity > 0)
        {
            S.printStats();
            printf("\n");
        }

        printf(ret == l_True ? "SATISFIABLE\n" : ret == l_False ? "UNSATISFIABLE\n"
                                                                : "INDETERMINATE\n");

        if (verify && ret == l_True)
        {
            bool check = S.verifySolution();
            if (check)
            {
                if (S.verbosity > 0)
                {
                    printf("Result is verified. OK\n");
                }
            }
            else
            {
                if (S.verbosity > 0)
                {
                    printf("Verifying FAILED\n");
                }

                exit(1);
            }
        }

        if (res != NULL)
        {
            if (ret == l_True)
            {
                fprintf(res, "SAT\n");
                size_t n_inputs = csat_instance->getInputGates().size();
                for (size_t i = 0; i < n_inputs; i++)
                {
                    if (S.model[i] != l_Undef)
                    {
                        fprintf(res, "%s%s%d", (i == 0) ? "" : " ", (S.model[i] == l_True) ? "" : "-", i + 1);
                    }
                }

                fprintf(res, " 0\n");
            }
            else if (ret == l_False)
            {
                fprintf(res, "UNSAT\n");
            }
            else
            {
                fprintf(res, "INDET\n");
            }

            fclose(res);
        }

#ifdef NDEBUG
        exit(ret == l_True ? 10 : ret == l_False ? 20
                                                 : 0); // (faster than "return", which will invoke the destructor for 'Solver')
#else
        return (ret == l_True ? 10 : ret == l_False ? 20
                                                    : 0);
#endif
    }
    catch (OutOfMemoryException &)
    {
        printf("===============================================================================\n");
        printf("INDETERMINATE\n");
        exit(0);
    }
}
