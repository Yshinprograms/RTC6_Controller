#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <thread>
#include <chrono>

#include "Rtc6Communicator.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include "RtcApiWrapper.h"
#include "DisplayUI.h"
#include "OvfParser.h" // Unchanged

#include "open_vector_format.pb.h" // Unchanged

void processOvfJob(Rtc6Communicator& rtcComm, OvfParser& parser) {
    if (!rtcComm.isSuccessfullySetup()) {
        std::cerr << "Aborting geometry processing: Board not ready." << std::endl;
        return;
    }

    const auto jobShell = parser.getJobShell();
    const int num_layers = parser.getNumberOfWorkPlanes();

    if (num_layers == 0) {
        std::cerr << "Aborting geometry processing: No layers to process." << std::endl;
        return;
    }

    // NEW: Define a constant for the physical recoating delay in milliseconds.
    // This simulates the build plate lowering and the powder recoater arm moving.
    const int recoating_delay_ms = 5000; // 5 seconds

    std::cout << "\n---------- Processing " << num_layers << " Layers from OVF Job ----------" << std::endl;

    RtcApiWrapper rtcApi;
    ListHandler listHandler(rtcComm, rtcApi);
    GeometryHandler geoHandler(listHandler);

    UINT lastListExecuted = 0;

    for (int i = 0; i < num_layers; ++i) {
        const auto work_plane = parser.getWorkPlane(i);
        UINT listToFill = listHandler.getCurrentFillListId();

        // This whole block (preparing the list) happens CONCURRENTLY while the previous layer is printing.
        std::cout << "\n[MAIN] Concurrently preparing Layer " << work_plane.work_plane_number()
            << " (Z=" << work_plane.z_pos_in_mm() << "mm) on List " << listToFill << "..." << std::endl;
        listHandler.beginListPreparation();
        for (const auto& block : work_plane.vector_blocks()) {
            try {
                const auto& params = jobShell.marking_params_map().at(block.marking_params_key());
                geoHandler.processVectorBlock(block, params);
            }
            catch (const std::out_of_range& e) {
                std::cerr << "WARNING: Marking params key " << block.marking_params_key()
                    << " not found. Skipping vector block." << std::endl;
            }
        }
        listHandler.endListPreparation();

        // Now, we wait for the PREVIOUS layer's hardware task to finish.
        if (lastListExecuted != 0) {
            std::cout << "[MAIN] Waiting for previous layer on List " << lastListExecuted << " to finish laser marking..." << std::endl;
            while (listHandler.isListBusy(lastListExecuted)) {
                // In a real app, this might have a timeout. For now, we wait indefinitely.
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            std::cout << "[MAIN] List " << lastListExecuted << " is now free." << std::endl;

            // NEW: Add the simulated delay for the physical recoating process.
            // This happens AFTER the previous layer's laser work is done, and BEFORE the next layer's begins.
            std::cout << "[MAIN] Simulating " << recoating_delay_ms << "ms for powder bed recoating..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(recoating_delay_ms));
        }

        std::cout << "[MAIN] Executing Layer " << work_plane.work_plane_number() << " on List " << listToFill << "." << std::endl;
        listHandler.executeCurrentListAndCycle();
        lastListExecuted = listToFill;
    }

    if (lastListExecuted != 0) {
        std::cout << "\n[MAIN] All lists prepared. Waiting for final layer execution on List " << lastListExecuted << "..." << std::endl;
        while (listHandler.isListBusy(lastListExecuted)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
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