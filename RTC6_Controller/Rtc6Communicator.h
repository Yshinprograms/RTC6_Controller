// Rtc6Communicator.h
#pragma once

#include <string>
#include "RTC6impl.h"

class Rtc6Communicator {
public:
    Rtc6Communicator(UINT boardId = 1); // Default to board 1
    ~Rtc6Communicator();
	bool initializeAndShowBoardInfo();
    bool connectAndSetupBoard();
    void printBoardVersions() const;
    void printBoardSerialNumber() const;
    void checkGlobalErrorStatus() const;
    bool isSuccessfullySetup() const;

private:
    // --- Private Member Variables ---
    UINT m_boardId;                 // Stores the ID of the RTC6 board to control
    UINT m_selectedCardNo;          // Stores the card number returned by select_rtc
    bool m_isDllInitialized;        // Tracks if init_rtc6_dll() was called
    bool m_isBoardSelected;         // Tracks if select_rtc() was successful
    bool m_isFirmwareLoaded;        // Tracks if load_program_file() was successful
    bool m_successfullySetup;       // Overall status of the setup process

    // --- Private Helper Methods ---
    bool initializeDll();
    UINT countCards();
    bool selectBoard();
    bool loadFirmware();
    bool checkError(const std::string& commandName, UINT rtcError) const;
    bool checkError(const std::string& commandName) const; // Overload to get last error internally
};