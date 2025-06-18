#include "PrintController.h"
#include "DisplayUI.h"
#include "OvfParser.h"
#include "Rtc6Communicator.h"
#include "RtcApiWrapper.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_ovf_file>" << std::endl;
        return 1;
    }

    // 1. Create the configuration object.
    PrintJobConfig config;
    config.ovfFilePath = argv[1];
    config.recoatingDelayMs = 5000;

    // 2. --- COMPOSITION ROOT ---
    // Create all the long-lived service components here.
    DisplayUI ui;
    OvfParser parser;
    Rtc6Communicator communicator(1); // Assuming board #1
    RtcApiWrapper rtcApi;             // The handlers depend on the API wrapper.

    // Create the handlers, injecting their dependencies.
    ListHandler listHandler(communicator, rtcApi);
    GeometryHandler geoHandler(listHandler);

    try {
        // 3. Create the main controller, injecting ALL 6 dependencies.
        PrintController controller(communicator, parser, ui, listHandler, geoHandler, config);

        // 4. Run the application.
        controller.run();
    }
    catch (const std::exception& e) {
        std::cerr << "\nFATAL ERROR: An unhandled exception occurred: " << e.what() << std::endl;
        ui.promptForExit();
        return 1;
    }

    return 0;
}