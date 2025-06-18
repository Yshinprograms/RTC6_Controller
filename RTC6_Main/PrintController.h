#pragma once // This is the essential include guard.

// --- Standard Library Dependencies ---
#include <string>

// --- Project Dependencies ---
// The PrintController class declaration uses these types, so the header MUST include their definitions.
#include "Rtc6Communicator.h"
#include "OvfParser.h"
#include "DisplayUI.h"

// --- Configuration Struct ---
// A simple struct to hold all configuration for a print job.
// It's good practice to place this here since it's tightly coupled with the controller.
struct PrintJobConfig {
    std::string ovfFilePath;
    int recoatingDelayMs;
};


// --- Class Declaration ---
// This is the "contract" for the PrintController.
class PrintController {
public:
    // The constructor's signature uses other classes, which is why we included them above.
    PrintController(
        Rtc6Communicator& communicator,
        OvfParser& parser,
        DisplayUI& ui,
        const PrintJobConfig& config
    );

    // The single public entry point to start the job.
    void run();

private:
    // Private helper method to contain the main loop logic.
    void processOvfJob();

    // Store references to the components we depend on.
    Rtc6Communicator& m_communicator;
    OvfParser& m_parser;
    DisplayUI& m_ui;
    const PrintJobConfig& m_config;
};