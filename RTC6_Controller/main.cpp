#include "MachineConfig.h"
#include "PrintController.h"
#include "ConsoleUI.h"
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

    PrintJobConfig config;
    config.ovfFilePath = argv[1];
    config.recoatingDelayMs = MachineConfig::RECOATING_DELAY_MS;

    ConsoleUI ui;
    OvfParser parser;
    Rtc6Communicator communicator(1);
    RtcApiWrapper rtcApi;
    ListHandler listHandler(communicator, rtcApi);
    GeometryHandler geoHandler(listHandler);

    try {
        PrintController controller(communicator, parser, ui, listHandler, geoHandler, config);

        controller.run();
    }
    catch (const std::exception& e) {
        std::cerr << "\nFATAL ERROR: An unhandled exception occurred: " << e.what() << std::endl;
        ui.promptForExit();
        return 1;
    }

    return 0;
}