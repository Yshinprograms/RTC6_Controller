#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

#include "Rtc6Communicator.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include "RtcApiWrapper.h"
#include "DisplayUI.h"
#include "OvfParser.h"

#include "open_vector_format.pb.h"

// The function is renamed and its signature is changed to accept the Protobuf Job object.
void processOvfJob(Rtc6Communicator& rtcComm, const open_vector_format::Job& job) {
    if (!rtcComm.isSuccessfullySetup() || job.work_planes().empty()) {
        std::cerr << "Aborting geometry processing: Board not ready or no layers to process." << std::endl;
        return;
    }
    std::cout << "\n---------- Processing " << job.work_planes_size() << " Layers from OVF Job ----------" << std::endl;

    RtcApiWrapper rtcApi;
    ListHandler listHandler(rtcComm, rtcApi);
    GeometryHandler geoHandler(listHandler);

    UINT lastListExecuted = 0;

    // The main loop now iterates directly over the Protobuf WorkPlane messages in the Job.
    for (const auto& work_plane : job.work_planes()) {
        UINT listToFill = listHandler.getCurrentFillListId();

        std::cout << "\n[MAIN] Concurrently preparing Layer " << work_plane.work_plane_number()
            << " (Z=" << work_plane.z_pos_in_mm() << "mm) on List " << listToFill << "..." << std::endl;

        listHandler.beginListPreparation();

        // The inner loop now iterates over Protobuf VectorBlocks.
        for (const auto& block : work_plane.vector_blocks()) {

            try {
                // Get the parameters for this block using its key from the job's map
                const auto& params = job.marking_params_map().at(block.marking_params_key());
                geoHandler.processVectorBlock(block, params);
            } catch (const std::out_of_range& e) {
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
    std::cout << "\n---------- All " << job.work_planes_size() << " Layers Processed ----------\n" << std::endl;
}

int main() {
    DisplayUI ui;
    ui.printWelcomeMessage();

    std::cout << "\n---------- Parsing OVF File ----------" << std::endl;
    OvfParser parser;

    // CHANGE: Use an absolute path to your test file to avoid "file not found" errors.
    // Replace this with the actual path on your machine.
    std::string ovf_file_path = "C:\\Users\\pin20\\Downloads\\SIMTech_Internship\\RTC6_Controller\\RTC6_Controller\\test_ovf.ovf";

    // CHANGE: The parser now returns the full Protobuf Job object.
    open_vector_format::Job job = parser.parseFile(ovf_file_path);

    // CHANGE: The check for empty layers is now done on the protobuf object.
    if (job.work_planes().empty()) {
        std::cerr << "Could not parse or file is empty. Aborting." << std::endl;
    }
    else {
        std::cout << "Successfully parsed " << job.work_planes_size() << " layer(s) from OVF file." << std::endl;
    }

    Rtc6Communicator rtcCommunicator(1);
    if (rtcCommunicator.initializeAndShowBoardInfo()) {
        // CHANGE: Call the newly refactored function with the Job object.
        processOvfJob(rtcCommunicator, job);
    }

    ui.printGoodbyeMessage();
    ui.promptForExit();
    return 0;
}