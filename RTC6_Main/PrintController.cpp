/**
 * @file PrintController.cpp
 * @brief Implements the PrintController class, the central orchestrator for the OVF printing process.
 *
 * This class encapsulates the entire application logic, from parsing the input file
 * to managing the hardware communication and layer-by-layer processing loop. It follows
 * the Controller Pattern, acting as a coordinator for various specialized components
 * like the OvfParser and Rtc6Communicator.
 */

#include "PrintController.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include "RtcApiWrapper.h" // Required by ListHandler and GeometryHandler
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept> // For std::exception

 /**
  * @brief Constructs a new PrintController instance.
  * @param communicator A reference to an initialized Rtc6Communicator for hardware interaction.
  * @param parser A reference to an OvfParser to read job data.
  * @param ui A reference to a DisplayUI for user interaction.
  * @param config A constant reference to a PrintJobConfig struct holding all job settings.
  *
  * This constructor uses a member initializer list to store references to all its
  * dependencies. This is a classic example of Dependency Injection, which makes
  * the class highly testable and decoupled from concrete component creation.
  */
PrintController::PrintController(
    Rtc6Communicator& communicator,
    OvfParser& parser,
    DisplayUI& ui,
    const PrintJobConfig& config
) : m_communicator(communicator), m_parser(parser), m_ui(ui), m_config(config) {
}


/**
 * @brief The main public entry point to start the entire print job process.
 *
 * This method orchestrates the high-level application lifecycle:
 * 1. Displays a welcome message.
 * 2. Parses the OVF file specified in the configuration.
 * 3. Initializes the RTC6 hardware.
 * 4. If all prerequisites are met, it delegates to the private processOvfJob() method to run the main loop.
 * 5. Displays a goodbye message and prompts for exit.
 */
void PrintController::run() {
    m_ui.printWelcomeMessage();

    std::cout << "\n---------- Parsing OVF File ----------" << std::endl;
    if (!m_parser.openFile(m_config.ovfFilePath)) {
        std::cerr << "Could not parse OVF file. Aborting." << std::endl;
        m_ui.promptForExit();
        return;
    }

    std::cout << "Successfully opened and parsed OVF file." << std::endl;
    std::cout << "Found " << m_parser.getNumberOfWorkPlanes() << " layer(s) to process." << std::endl;

    if (m_communicator.initializeAndShowBoardInfo()) {
        // Delegate the core processing logic to the dedicated private method.
        processOvfJob();
    }

    m_ui.printGoodbyeMessage();
    m_ui.promptForExit();
}


/**
 * @brief Manages the core layer-by-layer processing loop.
 *
 * This method implements the primary "ping-pong" buffering strategy for efficient
 * hardware utilization. The core idea is to prepare the data for the next layer (N+1)
 * in one RTC6 list buffer while the hardware is concurrently busy executing the
 * laser marking for the current layer (N) from the other list buffer.
 *
 * The sequence for each layer is as follows:
 * 1. Prepare the geometry for the current layer and load it into the currently free RTC6 list.
 * 2. Wait for the laser marking of the PREVIOUS layer to complete.
 * 3. Simulate a physical recoating delay (powder bed movement).
 * 4. Command the RTC6 board to execute the newly prepared list for the current layer.
 * 5. Cycle the list buffers so the one we just started becomes the "busy" one.
 */
void PrintController::processOvfJob() {
    // Check for hardware readiness one last time.
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "Aborting geometry processing: Board not ready." << std::endl;
        return;
    }

    const auto jobShell = m_parser.getJobShell();
    const int num_layers = m_parser.getNumberOfWorkPlanes();

    if (num_layers == 0) {
        std::cerr << "Aborting geometry processing: No layers to process." << std::endl;
        return;
    }

    // Retrieve the recoating delay from the configuration struct.
    const int recoating_delay_ms = m_config.recoatingDelayMs;

    std::cout << "\n---------- Processing " << num_layers << " Layers from OVF Job ----------" << std::endl;

    // Instantiate the hardware-specific handlers.
    RtcApiWrapper rtcApi;
    ListHandler listHandler(m_communicator, rtcApi);
    GeometryHandler geoHandler(listHandler);

    UINT lastListExecuted = 0; // Tracks the list ID of the previously executed layer.

    // Main processing loop
    for (int i = 0; i < num_layers; ++i) {
        const auto work_plane = m_parser.getWorkPlane(i);
        UINT listToFill = listHandler.getCurrentFillListId();

        std::cout << "\n[CONTROLLER] Concurrently preparing Layer "
            << work_plane.work_plane_number()
            << " (Z=" << work_plane.z_pos_in_mm() << "mm) on List "
            << listToFill << "..." << std::endl;

        // --- Software Task: Prepare List ---
        listHandler.beginListPreparation();
        for (const auto& block : work_plane.vector_blocks()) {
            try {
                // Look up the laser parameters for this block from the JobShell's map.
                const auto& params = jobShell.marking_params_map().at(block.marking_params_key());
                geoHandler.processVectorBlock(block, params);
            }
            catch (const std::out_of_range& e) {
                // Gracefully handle cases where a marking key is specified but not defined.
                std::cerr << "WARNING: Marking params key " << block.marking_params_key()
                    << " not found. Skipping vector block." << std::endl;
            }
        }
        listHandler.endListPreparation();

        // --- Synchronization Point ---
        // Wait for the PREVIOUS hardware task to finish before proceeding.
        if (lastListExecuted != 0) {
            std::cout << "[CONTROLLER] Waiting for previous layer on List " << lastListExecuted << " to finish laser marking..." << std::endl;
            while (listHandler.isListBusy(lastListExecuted)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            std::cout << "[CONTROLLER] List " << lastListExecuted << " is now free." << std::endl;

            // --- Physical Process Simulation ---
            std::cout << "[CONTROLLER] Simulating " << recoating_delay_ms << "ms for powder bed recoating..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(recoating_delay_ms));
        }

        // --- Hardware Task: Execute List ---
        std::cout << "[CONTROLLER] Executing Layer " << work_plane.work_plane_number() << " on List " << listToFill << "." << std::endl;
        listHandler.executeCurrentListAndCycle();
        lastListExecuted = listToFill; // Remember which list we just started.
    }

    // --- Final Wait ---
    // After the loop, we must wait for the very last layer to finish execution.
    if (lastListExecuted != 0) {
        std::cout << "\n[CONTROLLER] All lists prepared. Waiting for final layer execution on List " << lastListExecuted << "..." << std::endl;
        while (listHandler.isListBusy(lastListExecuted)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    std::cout << "\n---------- All " << num_layers << " Layers Processed ----------\n" << std::endl;
}