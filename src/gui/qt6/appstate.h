#ifndef APPSTATE_H
#define APPSTATE_H

class AppState
{
public:
    static AppState& instance() {
        static AppState s;
        return s;
    }

    // Solver Methodology input states
    bool solverMethodologyOk = false;
    bool useCOMtoggled = false;
    bool useCOMMtoggled = false;

    // Input states
    bool inputsOk = false;
    bool surfaceInputOk = false;
    bool diurnalInputToggled = false;
    bool stabilityInputToggled = false;

    // Wind Input States
    bool windInputOk = false;
    bool domainAverageWindToggled = false;
    bool domainAverageWindOk = false;
    bool pointInitializationToggled = false;
    bool pointInitializationOk = false;
    bool weatherModelToggled = false;
    bool weatherModelOk = false;

    // All Inputs Ok
    bool solverReady = false;


private:
    AppState() {}                         // private constructor
    AppState(const AppState&) = delete;  // prevent copying
    AppState& operator=(const AppState&) = delete;
};

#endif // APPSTATE_H
