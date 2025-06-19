#include "PrintController.h"
#include "Rtc6Exception.h"
#include <thread>
#include <chrono>
#include <stdexcept>
#include <sstream>

PrintController::PrintController(
    InterfaceCommunicator& communicator,
    InterfaceOvfParser& parser,
    InterfaceUI& ui,
    InterfaceListHandler& listHandler,
    InterfaceGeometryHandler& geoHandler,
    const PrintJobConfig& config
) : m_communicator(communicator),
m_parser(parser),
m_ui(ui),
m_listHandler(listHandler),
m_geoHandler(geoHandler),
m_config(config) {
    // Constructor body is empty, all work is done in the member initializer list.
}

/**
 * @brief The main public entry point to start the core print job logic.
 */
void PrintController::run() {
    m_ui.displayMessage("--- Initializing Hardware ---");
    if (!m_communicator.connectAndSetupBoard()) {
        m_ui.displayError("Hardware initialization failed. Aborting print job.");
        return;
    }
    m_ui.displayMessage("Hardware successfully initialized.");

    m_ui.displayMessage("\n--- Parsing OVF File ---");
    if (!m_parser.openFile(m_config.ovfFilePath)) {
        m_ui.displayError("Could not parse OVF file: " + m_config.ovfFilePath);
        return;
    }

    std::stringstream ss;
    ss << "Successfully opened and parsed OVF file. Found " << m_parser.getNumberOfWorkPlanes() << " layer(s) to process.";
    m_ui.displayMessage(ss.str());

    processOvfJob();
}

/**
 * @brief The core layer-by-layer processing loop.
 */
void PrintController::processOvfJob() {
    const int num_layers = m_parser.getNumberOfWorkPlanes();
    if (num_layers == 0) {
        m_ui.displayMessage("No layers found in the file. Nothing to process.");
        return;
    }

    m_ui.displayMessage("\n--- Starting Layer Processing ---");

    UINT lastListExecuted = 0;

    for (int i = 0; i < num_layers; ++i) {
        const auto work_plane = m_parser.getWorkPlane(i);

        prepareLayer(work_plane, m_parser.getJobShell());
        waitForPreviousLayer(lastListExecuted);
        executeLayer(work_plane);

        lastListExecuted = m_listHandler.getLastExecutedListId();
    }

    waitForPreviousLayer(lastListExecuted);
    m_ui.displayMessage("\n--- All " + std::to_string(num_layers) + " Layers Processed ---");
}

void PrintController::prepareLayer(const open_vector_format::WorkPlane& workPlane, const open_vector_format::Job& jobShell) {
    std::string progressMsg = "Preparing geometry on List " + std::to_string(m_listHandler.getCurrentFillListId());
    m_ui.displayProgress(progressMsg, workPlane.work_plane_number(), m_parser.getNumberOfWorkPlanes());

    m_listHandler.beginListPreparation();
    for (const auto& block : workPlane.vector_blocks()) {
        const auto& params_map = jobShell.marking_params_map();
        auto it = params_map.find(block.marking_params_key());

        if (it != params_map.end()) {
            const auto& params = it->second;
            m_geoHandler.processVectorBlock(block, params);
        }
        else {
            std::stringstream error_ss;
            error_ss << "Marking params key " << block.marking_params_key()
                << " not found in JobShell map. Skipping vector block.";
            throw ConfigurationError(error_ss.str());
        }
    }
    m_listHandler.endListPreparation();
}

void PrintController::waitForPreviousLayer(UINT listId) {
    if (listId == 0) {
        return;
    }

    m_ui.displayMessage("Waiting for previous layer on List " + std::to_string(listId) + " to finish...");
    while (m_listHandler.isListBusy(listId)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    m_ui.displayMessage("List " + std::to_string(listId) + " is now free.");

    std::stringstream recoat_ss;
    recoat_ss << "Simulating " << m_config.recoatingDelayMs << "ms for powder bed recoating...";
    m_ui.displayMessage(recoat_ss.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(m_config.recoatingDelayMs));
}

void PrintController::executeLayer(const open_vector_format::WorkPlane& workPlane) {
    std::stringstream exec_ss;
    exec_ss << "Executing Layer " << workPlane.work_plane_number() << " on List " << m_listHandler.getCurrentFillListId() << ".";
    m_ui.displayMessage(exec_ss.str());
    m_listHandler.executeCurrentListAndCycle();
}