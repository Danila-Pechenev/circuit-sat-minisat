// ---------- Main solver parameters ----------
//
// ccmin-mode (recommended 1 or 2). Default: 2
#define CCMIN_MODE 1
//
// rfirst. Default: 100
#define RFIRST 100
//
// Default polarity value. Default: true
#define DEFAULT_POLARITY_VALUE false
//
// --------------------------------------------
// --------------------------------------------
// ---------------- Heuristics ----------------
//
// Branching heuristics -----------------------
//
// Define to use back propagation
#define BACKPROP
#define COMPARE_BY_ACTIVITY false // Define to use activity-based back propagation
//
// Define to use jFrontiers-activity heuristic
// #define JFRONTIERS_ACTIVITY
//
// TODO: Define to use avoid-XOR back propagation
// #define AVOID_XOR_BACKPROP
//
// Define to start solving with Circuit-SAT heuristic
#define CSAT_HEURISTIC_START
#define DEFAULT_HEURISTIC_AFTER_N_RESTARTS 1 // Define to start solving with default Minisat heuristic after N restarts
// #define RESET_ACTIVITY                       // Define to reset activities after solving with Circuit-SAT heuristic or not
// #define RESET_POLARITY                       // Define to reset polarities after solving with Circuit-SAT heuristic or not
#define RESET_RESTARTS // Define to reset restarts after solving with Circuit-SAT heuristic or not
//
// Polarity heuristics ------------------------
//
// Define to use polarity initialization heuristic
#define POLARITY_INIT_HEURISTIC
//
// --------------------------------------------