// RTC6_Main/main.cpp

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

// Core Controller Components
#include "Rtc6Communicator.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include "RtcApiWrapper.h"
#include "DisplayUI.h"

// OVF Components
#include "OvfParser.h"
#include "ProcessData.h"


/**
 * @brief The main processing loop that orchestrates the layer-by-layer marking process.
 *
 * This function takes the parsed OVF data and uses a double-buffering ("ping-pong")
 * strategy to communicate with the RTC6 card. It prepares one layer in a background
 * list while the hardware is busy marking the current layer.
 *
 * @param rtcComm An initialized and ready Rtc6Communicator instance.
 * @param layers The vector of OvfLayer data parsed from the file.
*/
void processOvfLayers(Rtc6Communicator& rtcComm, const std::vector<OvfLayer>& layers) {
    if (!rtcComm.isSuccessfullySetup() || layers.empty()) {
        std::cerr << "Aborting geometry processing: Board not ready or no layers to process." << std::endl;
        return;
    }

    std::cout << "\n---------- Processing " << layers.size() << " Layers from OVF File ----------" << std::endl;

    // 1. Initialize our core handlers
    RtcApiWrapper rtcApi;
    ListHandler listHandler(rtcComm, rtcApi);
    GeometryHandler geoHandler(listHandler);

    UINT lastListExecuted = 0; // Keep track of the list we need to wait for

    // 2. The main loop: iterate through each layer from the OVF data
    for (size_t i = 0; i < layers.size(); ++i) {
        const auto& currentLayer = layers[i];
        UINT listToFill = listHandler.getCurrentFillListId();

        std::cout << "\n[MAIN] Concurrently preparing Layer " << i << " (Z=" << currentLayer.z_height_mm
            << "mm) on List " << listToFill << "..." << std::endl;

        // 3. Prepare all the commands for the current layer
        listHandler.beginListPreparation();
        for (const auto& polyline : currentLayer.polylines) {
            geoHandler.processPolyline(
                polyline.points,
                polyline.params.laserPowerPercent,
                polyline.params.markSpeed_mm_s,
                polyline.params.focusOffset_mm
            );
        }
        listHandler.endListPreparation();

        // 4. If this isn't the very first layer, we must wait for the previous layer to finish marking.
        if (i > 0) {
            std::cout << "[MAIN] Waiting for previous layer on List " << lastListExecuted << " to finish..." << std::endl;

            // THIS IS THE "SIGNAL": We poll until the hardware list is free.
            while (listHandler.isListBusy(lastListExecuted)) {
                Sleep(10); // The "small delay" you mentioned.
            }
            std::cout << "[MAIN] List " << lastListExecuted << " is now free." << std::endl;
        }

        // 5. Command the hardware to execute the list we just prepared.
        // This function also internally cycles the list handler to prepare for the *next* layer.
        std::cout << "[MAIN] Executing Layer " << i << " on List " << listToFill << "." << std::endl;
        listHandler.executeCurrentListAndCycle();
        lastListExecuted = listToFill;
    }

    // 6. After the loop, we must wait for the final layer to complete.
    if (lastListExecuted != 0) {
        std::cout << "\n[MAIN] All lists prepared. Waiting for final execution on List " << lastListExecuted << "..." << std::endl;
        while (listHandler.isListBusy(lastListExecuted)) {
            Sleep(50);
        }
    }

    std::cout << "\n---------- All " << layers.size() << " Layers Processed ----------\n" << std::endl;
}


int main() {
    DisplayUI ui;
    ui.printWelcomeMessage();

    // --- OVF Parsing ---
    std::cout << "\n---------- Parsing OVF File ----------" << std::endl;
    OvfParser parser;
    std::vector<OvfLayer> layers = parser.parseFile("C:\\Users\\pin20\\Downloads\\SIMTech_Internship\\RTC6_Controller\\RTC6_Controller\\test_ovf.ovf");

    if (layers.empty()) {
        std::cerr << "Could not parse or file is empty. Aborting." << std::endl;
    }
    else {
        std::cout << "Successfully parsed " << layers.size() << " layer(s) from OVF file." << std::endl;
    }

    // --- Board Communication and Processing ---
    Rtc6Communicator rtcCommunicator(1);
    if (rtcCommunicator.initializeAndShowBoardInfo()) {
        // Pass the parsed layers to our new processing function
        processOvfLayers(rtcCommunicator, layers);
    }

    ui.printGoodbyeMessage();
    ui.promptForExit();
    return 0;
}