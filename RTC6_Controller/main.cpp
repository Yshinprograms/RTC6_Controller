#include "PrintController.h"
#include "DisplayUI.h"
#include "OvfParser.h"
#include "Rtc6Communicator.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // 1. Handle command-line arguments (a robust addition).
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_ovf_file>" << std::endl;
        return 1;
    }

    // 2. Create the configuration object.
    PrintJobConfig config;
    config.ovfFilePath = argv[1];
    config.recoatingDelayMs = 5000;

    // 3. Create all the long-lived components.
    DisplayUI ui;
    OvfParser parser;
    Rtc6Communicator communicator(1); // Assuming board #1

    try {
        // 4. Create the main controller, injecting all dependencies.
        PrintController controller(communicator, parser, ui, config);

        // 5. Run the application.
        controller.run();
    }
    catch (const std::exception& e) {
        std::cerr << "\nFATAL ERROR: An unhandled exception occurred: " << e.what() << std::endl;
        ui.promptForExit();
        return 1;
    }

    return 0;
}