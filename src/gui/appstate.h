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
    bool isSolverMethodologyValid = false;
    bool isMassSolverToggled = false;
    bool isMomentumSolverToggled = false;

    // Input states
    bool isInputsValid = false;
    bool isSurfaceInputValid = false;
    bool isDiurnalInputToggled = false;
    bool isStabilityInputToggled = false;

    // Wind Input States
    bool isWindInputValid = false;
    bool isDomainAverageInitializationToggled = false;
    bool isDomainAverageWindInputTableValid = true;
    bool isDomainAverageInitializationValid = false;
    bool isPointInitializationToggled = false;
    bool isPointInitializationValid = false;
    bool isWeatherModelInitializationToggled = false;
    bool isWeatherModelInitializationValid = false;
    bool isShowAllTimeZonesSelected = false;
    bool isDisplayTimeZoneDetailsSelected = false;

    // All Inputs Ok
    bool isSolverReady = false;


private:
    AppState() {}                         // private constructor
    AppState(const AppState&) = delete;  // prevent copying
    AppState& operator=(const AppState&) = delete;
};

#endif // APPSTATE_H
