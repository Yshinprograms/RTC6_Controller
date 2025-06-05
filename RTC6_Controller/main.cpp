#include <iostream>
#include <string>
#include <Windows.h> // For Sleep function
#include <iostream>
#include <bitset>
#include "RTC6impl.h" 

// --- Helper Function for RTC6 Error Checking ---
// This function encapsulates error checking logic, making the main code cleaner and error reporting consistent.
// For R&D, detailed error messages are invaluable for debugging.
void CheckRtcError(UINT cardNo, const std::string& commandName) {
	UINT lastError = n_get_last_error(cardNo); // Get the last error for the specified board
	if (lastError != 0) {
		std::cerr << "ERROR: Command '" << commandName << "' failed on board " << cardNo << "." << std::endl;
		std::cerr << "       RTC6 Error Code: " << lastError << std::endl;

		// Optionally, reset the error for this board if you want to clear it after logging.
		// This is good practice to ensure subsequent error checks reflect new issues.
		n_reset_error(cardNo, lastError);
	}
}

int main()
{
	UINT errorCode;
	// Define the board ID for the RTC6 controller, always '1' for a single RTC6
	const UINT BOARD_ID = 1;

	std::cout << "---------- RTC6 Controller (Handshake Test) ----------" << std::endl;

	// Step 1: Initialize the RTC6 controller
	// *** IMPORTANT: Ensure sleep/standby mode on PC is DISABLED before initializing RTC6 ***
	// *** Use SleepMode.cmd in RTC6 Software Tools to deactivate ALL sleep and hibernation modes ***
	std::cout << "\n[Step 1] Initializing RTC6 controller..." << std::endl;
	errorCode = init_rtc6_dll();
	if (errorCode != 0) {
		std::cerr << "Error initializing RTC6 DLL: " << errorCode << std::endl;
		return errorCode;
	}
	std::cout << "RTC6 DLL initialized successfully." << std::endl;

	// Step 2: Count detected RTC6 boards
	UINT boardCount = rtc6_count_cards();
	std::cout << "\n[Step 2]  Detected " << boardCount << " RTC6 board(s)." << std::endl;
	if (boardCount == 0) {
		std::cerr << "No RTC6 boards detected. Exiting." << std::endl;
		free_rtc6_dll();
		return 1;
	}

	// Step 3: Select the RTC6 board for operation
	UINT selectedCard = 0;
	std::cout << "\n[Step 3] Attempting to select RTC6 board " << BOARD_ID << "..." << std::endl;
	selectedCard = select_rtc(selectedCard); // `select_rtc` returns the card number if successful, 0 if not.
	if (selectedCard == 0) {
		std::cerr << "FATAL ERROR: Failed to select RTC6 board. Check card number or access rights." << std::endl;
		CheckRtcError(selectedCard, "select_rtc");
		free_rtc6_dll();
		return 1;
	}
	selectedCard = BOARD_ID;
	std::cout << "RTC6 board " << selectedCard << " selected successfully." << std::endl;

	// Step 4: Load the RTC6 DSP firmware (Ensure out, rbf, and dat file are in executable directory)
	std::cout << "\n[Step 4] Loading RTC6 DSP firmware..." << std::endl;
	errorCode = load_program_file(0);
	if (errorCode != 0) {
		std::cerr << "FATAL ERROR: Failed to load DSP program file. Error Code: " << errorCode << std::endl;
		CheckRtcError(selectedCard, "load_program_file");
		std::cerr << "       Ensure RTC6OUT.out, RTC6RBF.rbf, RTC6DAT.dat are in the executable directory and are compatible with DLL." << std::endl;
		free_rtc6_dll();
		return 1;
	}
	std::cout << "DSP program file loaded successfully." << std::endl;

	// Step 5: Get RTC firmware version
	UINT rtcVersion = get_rtc_version();
	CheckRtcError(selectedCard, "get_rtc_version"); // Always check errors!
	std::cout << "\n[Step 5] Board " << selectedCard << " RTC Firmware Version: " << rtcVersion << std::endl;

	// Step 6: Get BIOS version
	UINT biosVersion = get_bios_version();
	CheckRtcError(selectedCard, "get_bios_version");
	std::cout << "\n[Step 6] Board " << selectedCard << " BIOS Version: " << biosVersion << std::endl;

	// Step 7: Get board serial number
	UINT serialNumber = get_serial_number();
	CheckRtcError(selectedCard, "n_get_serial_number");
	std::cout << "\n[Step 7] Board " << selectedCard << " Serial Number: " << serialNumber << std::endl;

	// Check for any accumulated errors that might not have been caught by individual `CheckRtcError` calls.
	UINT32 accumulatedError = get_error();
	if (accumulatedError != 0) {
		std::cerr << "WARNING: Accumulated errors on board " << selectedCard << ". Error Code: " << std::bitset<32>(accumulatedError) << std::endl;
		std::cout << "       Bit 3 will be set if there is no program uploaded onto RTC6" << std::endl;
	}

	std::cout << "\nRTC6 Handshake successful! Board is ready for further commands." << std::endl;

	// Keep console open to see output
	std::cout << "\nPress Enter to exit..." << std::endl;
	std::cin.ignore();
	std::cin.get();

	// Step 8: Release RTC6 DLL resources
	std::cout << "Releasing RTC6 DLL resources..." << std::endl;
	free_rtc6_dll();
	std::cout << "Resources released. Program exiting." << std::endl;

	return 0;
}