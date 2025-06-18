#pragma once

#include <string>
#include "Rtc6Communicator.h"
#include "OvfParser.h"
#include "DisplayUI.h"
#include "ListHandler.h"
#include "GeometryHandler.h"

struct PrintJobConfig { 
    std::string ovfFilePath;
	int recoatingDelayMs;
};

class PrintController {
public:
    // The constructor now accepts the handlers.
    PrintController(
        Rtc6Communicator& communicator,
        OvfParser& parser,
        DisplayUI& ui,
        ListHandler& listHandler,
        GeometryHandler& geoHandler,
        const PrintJobConfig& config
    );

    void run();

private:
    void processOvfJob();

    void prepareLayer(const open_vector_format::WorkPlane& workPlane, const open_vector_format::Job& jobShell);
    void waitForPreviousLayer(UINT listId);
    void executeLayer(const open_vector_format::WorkPlane& workPlane);

    Rtc6Communicator& m_communicator;
    OvfParser& m_parser;
    DisplayUI& m_ui;
    ListHandler& m_listHandler;
    GeometryHandler& m_geoHandler;
    const PrintJobConfig& m_config;
};