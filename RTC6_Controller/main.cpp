// main.cpp
#include <iostream>   // For console I/O
#include <memory>     // For std::unique_ptr
#include <vector>     // For std::vector
#include <Windows.h>  // For Sleep function (Windows-specific)

// --- Include all necessary headers from our layered architecture ---
#include "Rtc6Common.h"               // Custom exceptions, Point struct, common constants
#include "IRTCAPIDriver.h" // Low-level driver interface
#include "RTCAPIDriver.h" // Concrete low-level driver implementation
#include "IListCommandBuilder.h" // List command builder interface
#include "ListCommandBuilder.h" // Concrete list command builder
#include "ILaserControl.h" // Laser control interface
#include "LaserControl.h" // Concrete laser control
#include "RTC6Controller.h" // High-level RTC6 controller
#include "IMicroVectorProcessor.h" // Micro-vector processor interface
#include "MicroVectorProcessor.h" // Concrete micro-vector processor

int main() {
    std::cout << "---------- RTC6 L-PBF Research Application (Micro-Vector Demo) ----------" << std::endl;

    // The 'try-catch' block ensures that any Rtc6Exception or other runtime errors
    // are caught and reported gracefully, preventing unhandled crashes.
    try {
        const UINT BOARD_ID = 1; // The ID of the RTC6 board to control.

        // --- Step 1: Instantiate all concrete implementations ---
        // std::unique_ptr is used for explicit ownership and automatic resource management.
        // When these unique_ptrs go out of scope, their destructors will be called.
        std::unique_ptr<IRTCAPIDriver> driver = std::make_unique<RTCAPIDriver>(BOARD_ID);
        std::unique_ptr<IListCommandBuilder> listBuilder = std::make_unique<ListCommandBuilder>(driver.get());
        std::unique_ptr<ILaserControl> laserControl = std::make_unique<LaserControl>(driver.get());
        std::unique_ptr<IMicroVectorProcessor> microVectorProcessor = std::make_unique<MicroVectorProcessor>(listBuilder.get(), laserControl.get());

        // --- Step 2: Instantiate the high-level RTC6Controller, injecting its dependencies ---
        // RTC6Controller takes non-owning raw pointers because it doesn't own the lifecycle
        // of these specific implementations; 'main' does.
        RTC6Controller rtc6Controller(std::move(driver), std::move(listBuilder), std::move(laserControl));

        // --- Step 3: Initialize the RTC6 system ---
        // This high-level call handles DLL init, board selection, and firmware loading.
        rtc6Controller.InitializeAndLoadFirmware(BOARD_ID);

        // --- Step 4: Query and display board information ---
        UINT rtcVersion, biosVersion, serialNumber;
        rtc6Controller.QueryBoardInformation(BOARD_ID, rtcVersion, biosVersion, serialNumber);
        std::cout << "Board " << BOARD_ID << " RTC Firmware Version: " << rtcVersion << std::endl;
        std::cout << "Board " << BOARD_ID << " BIOS Version: " << biosVersion << std::endl;
        std::cout << "Board " << BOARD_ID << " Serial Number: " << serialNumber << std::endl;

        std::cout << "\nRTC6 System ready for marking operations." << std::endl;

        // --- Step 5: Configure Global Laser/Scanner Parameters ---
        // These are control commands, applied immediately to the board.
        // They also set the default values that ILaserControl will report for list commands.
        std::cout << "\n[Main] Configuring global laser and scanner parameters..." << std::endl;
        rtc6Controller.ConfigureGlobalLaser(
            0,                             // Laser Mode 0 (CO2 mode, or your specific mode)
            0x18,                          // Laser Control Signals (e.g., active-LOW for LASER1/LASER2 from manual examples)
            RTC6::DEFAULT_MARK_SPEED_BITS_MS, // Default mark speed in bits/ms
            RTC6::DEFAULT_JUMP_SPEED_BITS_MS  // Default jump speed in bits/ms
        );
        std::cout << "[Main] Global parameters set." << std::endl;

        // --- Step 6: Define a Sample Micro-Vector Path (e.g., a Square with Z variation) ---
        // Coordinates are in RTC6 "bits."
        LONG square_size_bits = RTC6::MillimetersToBits(10.0); // 10mm square converted to bits
        LONG start_x = RTC6::MillimetersToBits(-5.0); // -5mm converted to bits
        LONG start_y = RTC6::MillimetersToBits(-5.0); // -5mm converted to bits

        std::vector<Point> square_path = {
            Point(start_x, start_y, RTC6::MillimetersToBits(0.0)),             // Bottom-left (Z=0)
            Point(start_x + square_size_bits, start_y, RTC6::MillimetersToBits(0.0)),  // Bottom-right (Z=0)
            Point(start_x + square_size_bits, start_y + square_size_bits, RTC6::MillimetersToBits(2.0)), // Top-right (Z=+2mm)
            Point(start_x, start_y + square_size_bits, RTC6::MillimetersToBits(2.0)),   // Top-left (Z=+2mm)
            Point(start_x, start_y, RTC6::MillimetersToBits(0.0))             // Back to start (Z=0)
        };

        std::cout << "\n[Main] Path defined (5 points for a square with Z changes)." << std::endl;

        // --- Step 7: Build and Execute the Command List using MicroVectorProcessor ---
        const UINT MARKING_LIST_NO = 1;
        rtc6Controller.OpenList(MARKING_LIST_NO); // Prepare the board to receive list commands

        // Process the path into micro-vectors and add them to the list.
        // The MicroVectorProcessor uses the injected ListCommandBuilder and LaserControl.
        microVectorProcessor->ProcessMicroVectorPath(square_path);

        rtc6Controller.CloseList(); // Mark the end of the list for proper execution.

        std::cout << "[Main] List commands loaded onto RTC6 DSP. Executing list..." << std::endl;
        rtc6Controller.ExecuteList(MARKING_LIST_NO); // Start the DSP execution.

        // --- Step 8: Monitor List Execution Status (DSP side) ---
        // This is how you verify progress without physical output.
        std::cout << "[Main] Monitoring list execution (DSP status and pointer):" << std::endl;
        do {
            bool isBusy = rtc6Controller.IsListBusy();
            UINT currentPointer = rtc6Controller.GetListCurrentPointer();
            // Carriage return (\r) overwrites the current line for a dynamic progress display.
            std::cout << "\rList Status: " << (isBusy ? "BUSY" : "READY")
                << " | Current List Pointer: " << currentPointer << "            ";
            Sleep(100); // Pause for 100 milliseconds before polling again (Windows-specific).
        } while (rtc6Controller.IsListBusy());

        std::cout << "\n[Main] List execution finished on DSP." << std::endl;

    }
    catch (const Rtc6Exception& e) {
        std::cerr << "CRITICAL ERROR: Caught RTC6 Exception: " << e.what() << std::endl;
        // The unique_ptrs will handle resource cleanup, so no manual free_rtc6_dll().
        return static_cast<int>(e.getErrorCode()); // Return the specific RTC6 error code.
    }
    catch (const std::runtime_error& e) {
        std::cerr << "CRITICAL ERROR: Caught General Runtime Exception: " << e.what() << std::endl;
        return 1; // Generic non-RTC6 specific error code.
    }

    std::cout << "\nProgram execution complete. Press Enter to exit..." << std::endl;
    std::cin.ignore(); // Consume any leftover newline character.
    std::cin.get();    // Wait for user input before closing console.

    return 0; // Indicate successful program execution.
}