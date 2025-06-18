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
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

 /**
  * @brief Constructs a new PrintController instance.
  * @param communicator A reference to an initialized Rtc6Communicator for hardware interaction.
  * @param parser A reference to an OvfParser to read job data.
  * @param ui A reference to a DisplayUI for user interaction.
  * @param listHandler A reference to a ListHandler to manage RTC6 list buffers.
  * @param geoHandler A reference to a GeometryHandler to process vector data.
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
    ListHandler& listHandler,
    GeometryHandler& geoHandler,
    const PrintJobConfig& config
) : m_communicator(communicator),
m_parser(parser),
m_ui(ui),
m_listHandler(listHandler),
m_geoHandler(geoHandler),
m_config(config) {
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
        processOvfJob();
    }

    m_ui.printGoodbyeMessage();
    m_ui.promptForExit();
}


/**
 * @brief The refactored core processing loop.
 * This method now operates at a single, high level of abstraction, delegating
 * all work to specialized private helper methods. Its responsibility is to
 * iterate through the layers and coordinate the major steps of the print strategy.
 */
void PrintController::processOvfJob() {
    const int num_layers = m_parser.getNumberOfWorkPlanes();
    if (num_layers == 0) {
        std::cerr << "Aborting geometry processing: No layers to process." << std::endl;
        return;
    }

    std::cout << "\n---------- Processing " << num_layers << " Layers ----------" << std::endl;

    UINT lastListExecuted = 0;

    for (int i = 0; i < num_layers; ++i) {
        const auto work_plane = m_parser.getWorkPlane(i);

        // Each step in the process is now a single, readable function call.
        prepareLayer(work_plane, m_parser.getJobShell());
        waitForPreviousLayer(lastListExecuted);
        executeLayer(work_plane);

        // Get the ID of the list we just started so we can wait for it on the next iteration.
        lastListExecuted = m_listHandler.getLastExecutedListId();
    }

    // After the loop, we must wait for the very last layer to finish execution.
    waitForPreviousLayer(lastListExecuted);
    std::cout << "\n---------- All " << num_layers << " Layers Processed ----------\n" << std::endl;
}


// =================================================================================
// === PRIVATE HELPER METHODS (LOWER LEVEL OF ABSTRACTION) =========================
// =================================================================================

/**
 * @brief Prepares all geometry for a given WorkPlane and loads it into the free RTC list.
 * This method encapsulates the "software task" part of the ping-pong buffer strategy.
 * @param workPlane The WorkPlane object containing the geometry to process.
 * @param jobShell The Job object containing the parameter map.
 */
void PrintController::prepareLayer(const open_vector_format::WorkPlane& workPlane, const open_vector_format::Job& jobShell) {
    std::cout << "\n[CONTROLLER] Preparing Layer " << workPlane.work_plane_number() << " on List " << m_listHandler.getCurrentFillListId() << "..." << std::endl;

    m_listHandler.beginListPreparation();
    for (const auto& block : workPlane.vector_blocks()) {
        try {
            const auto& params = jobShell.marking_params_map().at(block.marking_params_key());
            m_geoHandler.processVectorBlock(block, params);
        }
        catch (const std::out_of_range& e) {
            std::cerr << "WARNING: Marking params key " << block.marking_params_key()
                << " not found in JobShell map. Skipping vector block." << std::endl;
        }
    }
    m_listHandler.endListPreparation();
}

/**
 * @brief Blocks execution until the specified list ID is no longer busy.
 * This method is the primary synchronization point in the processing loop. It also
 * injects the simulated delay for the physical recoating process.
 * @param listId The ID of the RTC6 list to monitor. If 0, the method returns immediately.
 */
void PrintController::waitForPreviousLayer(UINT listId) {
    // If listId is 0, it's the first layer, so there's nothing to wait for.
    if (listId == 0) {
        return;
    }

    std::cout << "[CONTROLLER] Waiting for previous layer on List " << listId << " to finish laser marking..." << std::endl;
    while (m_listHandler.isListBusy(listId)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << "[CONTROLLER] List " << listId << " is now free." << std::endl;

    // The recoating delay happens AFTER the previous layer's laser work is verifiably finished.
    std::cout << "[CONTROLLER] Simulating " << m_config.recoatingDelayMs << "ms for powder bed recoating..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(m_config.recoatingDelayMs));
}

/**
 * @brief Commands the RTC board to execute the currently prepared list and cycles the buffers.
 * This method encapsulates the "hardware task" part of the ping-pong buffer strategy.
 * @param workPlane The WorkPlane object being executed (used for logging).
 */
void PrintController::executeLayer(const open_vector_format::WorkPlane& workPlane) {
    std::cout << "[CONTROLLER] Executing Layer " << workPlane.work_plane_number() << " on List " << m_listHandler.getCurrentFillListId() << "." << std::endl;
    m_listHandler.executeCurrentListAndCycle();
}