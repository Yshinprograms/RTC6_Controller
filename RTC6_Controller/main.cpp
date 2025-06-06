#include <iostream>
#include <string>
#include <vector>
#include <Windows.h> // For Sleep()

// Include all our project headers
#include "Rtc6Communicator.h"
#include "DisplayUI.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include "Geometry.h"

// This function now demonstrates the full ping-pong buffer workflow.
void processTwoLayerSquare(Rtc6Communicator& rtcComm) {
    if (!rtcComm.isSuccessfullySetup()) {
        std::cerr << "Aborting geometry processing due to board setup failure." << std::endl;
        return;
    }

    std::cout << "\n---------- Processing 2-Layer Square (Ping-Pong Buffer Demo) ----------" << std::endl;

    // --- 1. Setup ---
    // Create the handlers, linking them together.
    ListHandler listHandler(rtcComm);
    GeometryHandler geoHandler(listHandler);

    // Define the geometry for two separate layers.
    std::vector<Point> layer1_square = {
        {10.0, 10.0}, {20.0, 10.0}, {20.0, 20.0}, {10.0, 20.0}, {10.0, 10.0}
    };
    std::vector<Point> layer2_square = {
        {11.0, 11.0}, {21.0, 11.0}, {21.0, 21.0}, {11.0, 21.0}, {11.0, 11.0}
    };

    // --- 2. Prepare and Execute Layer 1 ---
    std::cout << "\n[MAIN] Preparing List 1 for Layer 1..." << std::endl;
    listHandler.beginListPreparation();
    geoHandler.processPolyline(layer1_square, 50.0, 1000.0, 0.0);
    listHandler.endListPreparation();

    // Arm the hardware to automatically switch to the next list upon completion.
    listHandler.setupAutoChangeMode();

    std::cout << "\n[MAIN] Executing List 1. The hardware is now busy." << std::endl;
    listHandler.executeCurrentListAndCycle(); // Starts List 1, handler now targets List 2.

    // --- 3. Prepare Layer 2 CONCURRENTLY ---
    // While the hardware is busy with List 1, we immediately prepare List 2.
    std::cout << "\n[MAIN] Concurrently preparing List 2 for Layer 2..." << std::endl;

    // First, wait until List 2 is actually free (it should be instantly, but this is good practice).
    while (listHandler.isListBusy(2)) {
        std::cout << "  Waiting for List 2 to become available for filling..." << std::endl;
        Sleep(10);
    }

    listHandler.beginListPreparation(); // This now targets List 2.
    geoHandler.processPolyline(layer2_square, 55.0, 1200.0, 0.1); // Different params for layer 2
    listHandler.endListPreparation();

    std::cout << "[MAIN] List 2 is now loaded and ready. The hardware will switch to it automatically." << std::endl;

    // --- 4. Wait for ALL processing to finish ---
    // We need to wait for both List 1 and List 2 to complete.
    std::cout << "\n[MAIN] Waiting for all hardware processing to complete..." << std::endl;

    // Wait for List 1 to finish.
    while (listHandler.isListBusy(1)) {
        std::cout << "  Hardware is processing List 1..." << std::endl;
        Sleep(50);
    }
    std::cout << "  List 1 finished. Hardware has auto-switched to List 2." << std::endl;

    // Now, wait for List 2 to finish.
    while (listHandler.isListBusy(2)) {
        std::cout << "  Hardware is processing List 2..." << std::endl;
        Sleep(50);
    }
    std::cout << "  List 2 finished." << std::endl;

    std::cout << "\n---------- 2-Layer Square Finished ----------\n" << std::endl;
}

int main() {
    DisplayUI ui;
    ui.printWelcomeMessage();

    Rtc6Communicator rtcCommunicator(1);
    if (rtcCommunicator.initializeAndShowBoardInfo()) {
        // If the board is ready, run the ping-pong buffer demo.
        processTwoLayerSquare(rtcCommunicator);
    }

    ui.printGoodbyeMessage();
    ui.promptForExit();
    return 0;
}