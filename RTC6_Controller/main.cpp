#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

#include "Rtc6Communicator.h"
#include "DisplayUI.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include "Geometry.h"

void processMultipleLayers(Rtc6Communicator& rtcComm, int numLayers) {
    if (!rtcComm.isSuccessfullySetup() || numLayers < 1) {
        std::cerr << "Aborting geometry processing: Board not ready or no layers to process." << std::endl;
        return;
    }

    std::cout << "\n---------- Processing " << numLayers << " Layers (Continuous Ping-Pong) ----------" << std::endl;

    ListHandler listHandler(rtcComm);
    GeometryHandler geoHandler(listHandler);

    // --- Step 1: Prepare and execute the VERY FIRST layer ---
    UINT listToFill = listHandler.getCurrentFillListId();
    std::cout << "\n[MAIN] Preparing initial List " << listToFill << " for Layer 1..." << std::endl;
    listHandler.beginListPreparation();
    std::vector<Point> geometryLayer1 = { {10, 10}, {20, 10}, {20, 20}, {10, 20}, {10, 10} };
    geoHandler.processPolyline(geometryLayer1, 50.0, 1000.0, 0.0);
    listHandler.endListPreparation();

    // Arm the first switch and execute.
    listHandler.setupAutoChangeMode();
    listHandler.executeCurrentListAndCycle();
    UINT lastListFilled = listToFill;

    // --- Step 2: Main processing loop for subsequent layers ---
    for (int i = 2; i <= numLayers; ++i) {
        listToFill = lastListFilled;
        UINT busyList = (listToFill == 1) ? 2 : 1;

        std::cout << "\n[MAIN] Hardware is processing Layer " << i - 1 << " on List " << busyList << "." << std::endl;
        std::cout << "[MAIN] Concurrently preparing List " << listToFill << " for Layer " << i << "..." << std::endl;

        std::vector<Point> nextLayer = { {10.0 + i, 10.0 + i}, {20.0 + i, 10.0 + i}, {20.0 + i, 20.0 + i}, {10.0 + i, 20.0 + i}, {10.0 + i, 10.0 + i} };

        listHandler.beginListPreparation();
        geoHandler.processPolyline(nextLayer, 50.0 + i, 1200.0 + (i * 50), 0.0); // Varying speed and power
        listHandler.endListPreparation();

        listHandler.reArmAutoChange();

        std::cout << "[MAIN] Waiting for List " << busyList << " to become free..." << std::endl;
        while (listHandler.isListBusy(busyList)) {
            Sleep(10);
        }
        std::cout << "[MAIN] List " << busyList << " is now free." << std::endl;

        lastListFilled = listToFill;
    }

    // --- Step 3: Wait for the final list to complete ---
    std::cout << "\n[MAIN] All lists prepared. Waiting for final execution on List " << lastListFilled << "..." << std::endl;
    while (listHandler.isListBusy(lastListFilled)) {
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