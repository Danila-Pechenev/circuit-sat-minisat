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
//
// Define to use jFrontiers-activity heuristic
// #define JFRONTIERS_ACTIVITY
//
// Define to use activity-based back propagation
// #define BACKPROP_ACTIVITY
//
// TODO: Define to use XOR back propagation
// #define XOR_BACKPROP
//
// Define to start solving with Circuit-SAT heuristic
#define CSAT_HEURISTIC_START
#define DEFAULT_HEURISTIC_AFTER_N_RESTARTS 1 // Start solving with default Minisat heuristic after N restarts
#define RESET_ACTIVITY                       // Reset activities after solving with Circuit-SAT heuristic or not
// #define RESET_POLARITY                       // Reset polarities after solving with Circuit-SAT heuristic or not
#define RESET_RESTARTS // Reset restarts after solving with Circuit-SAT heuristic or not
//
// Polarity heuristics ------------------------
//
// Define to use polarity initialization heuristic
#define POLARITY_INIT_HEURISTIC
//
// --------------------------------------------