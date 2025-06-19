#include "MachineConfig.h"
#include "PrintJobConfig.h"
#include "PrintController.h"
#include "ConsoleUI.h"
#include "OvfParser.h"
#include "Rtc6Communicator.h"
#include "RtcApiWrapper.h"
#include "ListHandler.h"
#include "GeometryHandler.h"
#include "Rtc6Exception.h"
#include <iostream>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <path_to_ovf_file>" << std::endl;
		return 1;
	}

	PrintJobConfig config;
	config.ovfFilePath = argv[1];
	config.recoatingDelayMs = MachineConfig::RECOATING_DELAY_MS;

	ConsoleUI ui;
	OvfParser parser;
	Rtc6Communicator communicator(1);
	RtcApiWrapper rtcApi;
	ListHandler listHandler(communicator, rtcApi);
	GeometryHandler geoHandler(listHandler);

	int exitCode = 0;

	try {
		ui.printWelcomeMessage();
		PrintController controller(communicator, parser, ui, listHandler, geoHandler, config);
		controller.run();
		ui.printGoodbyeMessage();
	}
	catch (const HardwareError& e) {
		ui.displayError("A fatal hardware error occurred: " + std::string(e.what()));
		exitCode = 1;
	}
	catch (const FileParseError& e) {
		ui.displayError("A fatal file error occurred: " + std::string(e.what()));
		exitCode = 1;
	}
	catch (const ConfigurationError& e) {
		ui.displayError("A fatal configuration error occurred: " + std::string(e.what()));
		exitCode = 1;
	}
	catch (const Rtc6Exception& e) {
		ui.displayError("An unexpected library error occurred: " + std::string(e.what()));
		exitCode = 1;
	}
	catch (const std::exception& e) {
		ui.displayError("An unexpected standard error occurred: " + std::string(e.what()));
		exitCode = 1;
	}

	ui.promptForExit();
	return exitCode;
}