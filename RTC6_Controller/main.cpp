#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

#include "Rtc6Communicator.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include "RtcApiWrapper.h" // Provides the "real" implementation of the RTC6 API calls.
#include "DisplayUI.h"
#include "Geometry.h"

// Demonstrates the continuous "ping-pong" buffering process for multiple layers.
// This function simulates a real L-PBF build by preparing one layer's data
// while the hardware is busy processing the previous layer.
void processMultipleLayers(Rtc6Communicator& rtcComm, int numLayers) {
    if (!rtcComm.isSuccessfullySetup() || numLayers < 1) {
        std::cerr << "Aborting geometry processing: Board not ready or no layers to process." << std::endl;
        return;
    }
    std::cout << "\n---------- Processing " << numLayers << " Layers (Continuous Ping-Pong) ----------" << std::endl;

    // --- Object Composition Root ---
    // Create the concrete implementations of the dependencies.
    RtcApiWrapper rtcApi;
    ListHandler listHandler(rtcComm, rtcApi);
    GeometryHandler geoHandler(listHandler);
    // --- End of Composition ---

    // --- Initial Layer Preparation (outside the loop) ---
    UINT listToFill = listHandler.getCurrentFillListId();
    std::cout << "\n[MAIN] Preparing initial List " << listToFill << " for Layer 1..." << std::endl;
    listHandler.beginListPreparation();
    std::vector<Point> geometryLayer1 = { {10, 10}, {20, 10}, {20, 20}, {10, 20}, {10, 10} };
    geoHandler.processPolyline(geometryLayer1, 50.0, 1000.0, 0.0);
    listHandler.endListPreparation();

    // Kick off the automated process.
    listHandler.setupAutoChangeMode();
    listHandler.executeCurrentListAndCycle();
    UINT lastListExecuted = listToFill;

    // --- Concurrent Processing Loop for Subsequent Layers ---
    for (int i = 2; i <= numLayers; ++i) {
        // The list that was just sent for execution is now the busy one.
        UINT busyList = lastListExecuted;
        // The list that the hardware will switch TO is the one we need to fill now.
        listToFill = listHandler.getCurrentFillListId();

        std::cout << "\n[MAIN] Hardware is processing Layer " << i - 1 << " on List " << busyList << "." << std::endl;
        std::cout << "[MAIN] Concurrently preparing List " << listToFill << " for Layer " << i << "..." << std::endl;

        // Prepare the next layer's data.
        std::vector<Point> nextLayer = { {10.0 + i, 10.0 + i}, {20.0 + i, 10.0 + i}, {20.0 + i, 20.0 + i}, {10.0 + i, 20.0 + i}, {10.0 + i, 10.0 + i} };
        listHandler.beginListPreparation();
        geoHandler.processPolyline(nextLayer, 50.0 + i, 1200.0 + (i * 50), 0.0);
        listHandler.endListPreparation();

        // Re-arm the auto-change trigger for the next transition.
        listHandler.reArmAutoChange();

        // Before the next iteration, we must wait for the hardware to finish the previous layer.
        // This ensures we don't try to overwrite a list buffer that is still being read.
        std::cout << "[MAIN] Waiting for List " << busyList << " to become free..." << std::endl;
        while (listHandler.isListBusy(busyList)) {
            Sleep(10); // Polling delay to prevent maxing out a CPU core.
        }
        std::cout << "[MAIN] List " << busyList << " is now free." << std::endl;

        lastListExecuted = listToFill;
    }

    // After the loop, wait for the very last layer to finish processing.
    std::cout << "\n[MAIN] All lists prepared. Waiting for final execution on List " << lastListExecuted << "..." << std::endl;
    while (listHandler.isListBusy(lastListExecuted)) {
        Sleep(50);
    }
    std::cout << "\n---------- All " << numLayers << " Layers Processed ----------\n" << std::endl;
}

int main() {
    DisplayUI ui;
    ui.printWelcomeMessage();

    Rtc6Communicator rtcCommunicator(1);
    if (rtcCommunicator.initializeAndShowBoardInfo()) {
        processMultipleLayers(rtcCommunicator, 5);
    }

    ui.printGoodbyeMessage();
    ui.promptForExit();
    return 0;
}