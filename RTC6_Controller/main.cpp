#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

#include "Rtc6Communicator.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include "RtcApiWrapper.h"
#include "DisplayUI.h"
#include "OvfParser.h" // Unchanged

#include "open_vector_format.pb.h" // Unchanged

// =========================================================================
// CHANGE: The function signature is updated to accept the parser.
// It will use the parser to get the job shell and then fetch each layer.
void processOvfJob(Rtc6Communicator& rtcComm, OvfParser& parser) {
    if (!rtcComm.isSuccessfullySetup()) {
        std::cerr << "Aborting geometry processing: Board not ready." << std::endl;
        return;
    }

    // CHANGE: Get the job's shell (metadata) from the parser.
    const auto jobShell = parser.getJobShell();
    const int num_layers = parser.getNumberOfWorkPlanes();

    if (num_layers == 0) {
        std::cerr << "Aborting geometry processing: No layers to process." << std::endl;
        return;
    }

    std::cout << "\n---------- Processing " << num_layers << " Layers from OVF Job ----------" << std::endl;

    RtcApiWrapper rtcApi;
    ListHandler listHandler(rtcComm, rtcApi);
    GeometryHandler geoHandler(listHandler);

    UINT lastListExecuted = 0;

    // CHANGE: The main loop is now an index-based for loop.
    for (int i = 0; i < num_layers; ++i) {
        // CHANGE: Fetch the full WorkPlane object for the current layer ON DEMAND.
        const auto work_plane = parser.getWorkPlane(i);

        UINT listToFill = listHandler.getCurrentFillListId();

        std::cout << "\n[MAIN] Concurrently preparing Layer " << work_plane.work_plane_number()
            << " (Z=" << work_plane.z_pos_in_mm() << "mm) on List " << listToFill << "..." << std::endl;

        listHandler.beginListPreparation();

        // The inner loop logic remains the same, operating on the fetched work_plane.
        for (const auto& block : work_plane.vector_blocks()) {
            try {
                // The marking parameters are retrieved from the jobShell we got earlier.
                const auto& params = jobShell.marking_params_map().at(block.marking_params_key());
                geoHandler.processVectorBlock(block, params);
            }
            catch (const std::out_of_range& e) {
                std::cerr << "WARNING: Marking params key " << block.marking_params_key()
                    << " not found. Skipping vector block." << std::endl;
            }
        }

        listHandler.endListPreparation();

        if (lastListExecuted != 0) {
            std::cout << "[MAIN] Waiting for previous layer on List " << lastListExecuted << " to finish..." << std::endl;
            while (listHandler.isListBusy(lastListExecuted)) {
                Sleep(10);
            }
            std::cout << "[MAIN] List " << lastListExecuted << " is now free." << std::endl;
        }

        std::cout << "[MAIN] Executing Layer " << work_plane.work_plane_number() << " on List " << listToFill << "." << std::endl;
        listHandler.executeCurrentListAndCycle();
        lastListExecuted = listToFill;
    }

    if (lastListExecuted != 0) {
        std::cout << "\n[MAIN] All lists prepared. Waiting for final execution on List " << lastListExecuted << "..." << std::endl;
        while (listHandler.isListBusy(lastListExecuted)) {
            Sleep(50);
        }
    }
    std::cout << "\n---------- All " << num_layers << " Layers Processed ----------\n" << std::endl;
}

int main() {
    DisplayUI ui;
    ui.printWelcomeMessage();

    std::cout << "\n---------- Parsing OVF File ----------" << std::endl;
    OvfParser parser;
    std::string ovf_file_path = "C:\\Users\\pin20\\Downloads\\SIMTech_Internship\\RTC6_Controller\\RTC6_Controller\\cube_tenByTen.ovf";

    // CHANGE: Call the new openFile() method and check its boolean result.
    if (!parser.openFile(ovf_file_path)) {
        std::cerr << "Could not parse OVF file. Aborting." << std::endl;
    }
    else {
        // CHANGE: Use getJobShell() to confirm layers exist.
        std::cout << "Successfully opened and parsed LUTs from OVF file." << std::endl;
        // CHANGE: Use the new method to get the correct layer count.
        const int num_layers = parser.getNumberOfWorkPlanes();
        std::cout << "Found " << num_layers << " layer(s) to process." << std::endl;

        Rtc6Communicator rtcCommunicator(1);
        if (rtcCommunicator.initializeAndShowBoardInfo()) {
            processOvfJob(rtcCommunicator, parser);
        }
    }

    ui.printGoodbyeMessage();
    ui.promptForExit();
    return 0;
}