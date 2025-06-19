#pragma once

#include "InterfacePrintController.h"
#include "InterfaceCommunicator.h"
#include "InterfaceOvfParser.h"
#include "InterfaceListHandler.h"
#include "InterfaceGeometryHandler.h"
#include "InterfaceUI.h"

#include "PrintJobConfig.h"

class PrintController : public InterfacePrintController {
public:
    // --- Constructor now accepts INTERFACES ---
    PrintController(
        InterfaceCommunicator& communicator,
        InterfaceOvfParser& parser,
        InterfaceUI& ui,
        InterfaceListHandler& listHandler,
        InterfaceGeometryHandler& geoHandler,
        const PrintJobConfig& config);

    void run() override;

private:
    void processOvfJob();
    void prepareLayer(const open_vector_format::WorkPlane& workPlane, const open_vector_format::Job& jobShell);
    void waitForPreviousLayer(UINT listId);
    void executeLayer(const open_vector_format::WorkPlane& workPlane);

    // --- Member variables are INTERFACES ---
    InterfaceCommunicator& m_communicator;
    InterfaceOvfParser& m_parser;
    InterfaceUI& m_ui;
    InterfaceListHandler& m_listHandler;
    InterfaceGeometryHandler& m_geoHandler;
    const PrintJobConfig& m_config;
};