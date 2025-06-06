// Rtc6Communicator.cpp
#include "Rtc6Communicator.h" // Corresponding header file
#include <iostream>             // For std::cout, std::cerr, std::endl
#include <iomanip>              // For std::hex (to print error codes in hex)
#include <string>               // For std::string
#include <bitset>               // For std::bitset (to print error codes in binary)
// #include <Windows.h>         // Only if Sleep() or other Windows-specific functions are directly used here

// --- Constructor ---
Rtc6Communicator::Rtc6Communicator(UINT boardId)
    : m_boardId(boardId),
    m_selectedCardNo(0), // Initialize to 0, meaning no card selected/selection failed
    m_isDllInitialized(false),
    m_isBoardSelected(false),
    m_isFirmwareLoaded(false),
    m_successfullySetup(false) { 
    std::cout << "[Rtc6Communicator] Instance created for board ID: " << m_boardId << std::endl;
    std::cout << "[Rtc6Communicator] INFO: Ensure PC sleep/standby modes are DISABLED for RTC6 operation." << std::endl;
}

// --- Destructor ---
Rtc6Communicator::~Rtc6Communicator() {
    if (m_isDllInitialized) {
        std::cout << "[Rtc6Communicator] Releasing RTC6 DLL resources..." << std::endl;
        free_rtc6_dll();
    }
    std::cout << "[Rtc6Communicator] Instance destroyed." << std::endl;
}

// --- Private Helper: Error Checking ---
// This version takes an explicit error code.
bool Rtc6Communicator::checkError(const std::string& commandName, UINT rtcError) const {
    if (rtcError != 0) {
        std::cerr << "ERROR: [Rtc6Communicator] Command '" << commandName
            << "' failed on board context " << m_selectedCardNo // m_selectedCardNo might be 0 if select_rtc failed
            << "." << std::endl;
        std::cerr << "       RTC6 Error Code: " << rtcError << " (Decimal), 0x"
            << std::hex << rtcError << std::dec << " (Hex)" << std::endl; // Show in hex too
        // n_reset_error(m_selectedCardNo, rtcError); // Use with caution and understanding of RTC6 error handling
        return false; // Error occurred
    }
    return true; // No error
}

// This version retrieves the last error for the currently selected card (or global if appropriate).
bool Rtc6Communicator::checkError(const std::string& commandName) const {
    UINT cardContextForError = m_selectedCardNo;

    // For some very early commands, or if selection failed, m_selectedCardNo might be 0.
    // init_rtc6_dll and rtc6_count_cards errors are global, not card-specific.
    // select_rtc errors are also global until a card is successfully selected.
    if (commandName == "init_rtc6_dll" || commandName == "rtc6_count_cards" || (commandName == "select_rtc" && m_selectedCardNo == 0)) {
        cardContextForError = 0; // Use global error context
    }
    else if (!m_isBoardSelected) {
        // If board isn't selected, but we are past the initial commands, it's an issue.
        // However, get_last_error needs a card number. If m_selectedCardNo is 0, it refers to global error.
        std::cerr << "WARNING: [Rtc6Communicator] Attempting to get error for '" << commandName
            << "' but no board is actively selected. Checking global error." << std::endl;
        cardContextForError = 0;
    }
    // If m_selectedCardNo is valid (e.g., 1 after successful select_rtc(1)), use it.

    UINT lastError = n_get_last_error(cardContextForError);
    return checkError(commandName, lastError);
}

// --- Private Helper Methods Implementation ---
bool Rtc6Communicator::initializeDll() {
    std::cout << "\n[Rtc6Communicator] Initializing RTC6 DLL..." << std::endl;
    UINT errorCode = init_rtc6_dll(); // This is a global function, does not take cardNo
    if (errorCode != 0) { // init_rtc6_dll returns an error code directly
        checkError("init_rtc6_dll", errorCode); // Log it using our helper
        std::cerr << "       This can be due to missing RTC6DLLx64.dll, driver issues, or licensing problems." << std::endl;
        return false;
    }
    m_isDllInitialized = true;
    std::cout << "[Rtc6Communicator] RTC6 DLL initialized successfully." << std::endl;
    return true;
}

UINT Rtc6Communicator::countCards() {
    if (!m_isDllInitialized) {
        std::cerr << "ERROR: [Rtc6Communicator] Cannot count cards, DLL not initialized." << std::endl;
        return 0;
    }
    UINT count = rtc6_count_cards(); // Global function
    std::cout << "\n[Rtc6Communicator] Detected " << count << " RTC6 board(s)." << std::endl;
    if (count == 0) {
        // rtc6_count_cards itself doesn't set a last_error if it returns 0.
        // The "error" is simply that no cards are found.
        std::cerr << "       No RTC6 boards found. Ensure board is powered, connected via PCIe, and drivers are installed." << std::endl;
    }
    return count;
}

bool Rtc6Communicator::selectBoard() {
    if (!m_isDllInitialized) {
        std::cerr << "ERROR: [Rtc6Communicator] Cannot select board, DLL not initialized." << std::endl;
        return false;
    }

    std::cout << "\n[Rtc6Communicator] Attempting to select RTC6 board with intended ID " << m_boardId << "..." << std::endl;

    // select_rtc uses 1-based indexing for CardNo.
    // It returns the selected card number (which should match m_boardId if successful) or 0 on failure.
    UINT actualSelectedCard = select_rtc(m_boardId);

    if (actualSelectedCard == 0) {
        std::cerr << "FATAL ERROR: [Rtc6Communicator] Failed to select RTC6 board " << m_boardId << "." << std::endl;
        // For select_rtc failure, check global error (card 0) or error on the card we tried to select
        UINT errorVal = n_get_last_error(0); // Check global error first
        if (errorVal == 0 && m_boardId != 0) { // If no global error, check the specific card we aimed for
            errorVal = n_get_last_error(m_boardId);
        }
        checkError("select_rtc", errorVal != 0 ? errorVal : 1); // Pass errorVal, or 1 if no specific code but selection failed
        std::cerr << "       Check card number, physical connection, driver conflicts, or access rights (e.g., run as admin if needed)." << std::endl;
        m_selectedCardNo = 0; // Ensure it's 0 on failure
        m_isBoardSelected = false;
        return false;
    }

    m_selectedCardNo = actualSelectedCard; // Store the card number returned by the API
    std::cout << "[Rtc6Communicator] RTC6 board " << m_selectedCardNo << " selected successfully." << std::endl;
    m_isBoardSelected = true;
    return true;
}

bool Rtc6Communicator::loadFirmware() {
    if (!m_isBoardSelected) {
        std::cerr << "ERROR: [Rtc6Communicator] Cannot load firmware, no board selected." << std::endl;
        return false;
    }

    std::cout << "\n[Rtc6Communicator] Loading RTC6 DSP firmware onto board " << m_selectedCardNo << "..." << std::endl;
    // load_program_file(0) loads from the executable's directory for the currently selected card.
    // The SDK mentions Path=NULL for current directory, which is equivalent to Path=0 for the C API.
    UINT errorCode = load_program_file(nullptr); // Or load_program_file(0)

    if (errorCode != 0) { // load_program_file returns an error code directly
        checkError("load_program_file", errorCode);
        std::cerr << "       Ensure RTC6OUT.out, RTC6RBF.rbf, RTC6DAT.dat are in the executable directory," << std::endl;
        std::cerr << "       are not corrupted, and are compatible with the RTC6DLLx64.dll version." << std::endl;
        return false;
    }

    m_isFirmwareLoaded = true;
    std::cout << "[Rtc6Communicator] DSP program file loaded successfully onto board " << m_selectedCardNo << "." << std::endl;
    return true;
}

// --- Public Methods Implementation ---
bool Rtc6Communicator::connectAndSetupBoard() {
    m_successfullySetup = false; // Reset status for a fresh attempt

    if (!initializeDll()) {
        return false;
    }

    UINT cardCount = countCards();
    if (cardCount == 0) {
        return false;
    }
    if (m_boardId > cardCount && cardCount > 0) {
        std::cerr << "WARNING: [Rtc6Communicator] Requested board ID " << m_boardId
            << " is greater than detected card count " << cardCount
            << ". Defaulting to board 1, please check connections. " << std::endl;
        m_boardId = 1;
    }

    if (!selectBoard()) {
        return false;
    }

    if (!loadFirmware()) {
        return false;
    }

    std::cout << "\n[Rtc6Communicator] Core board setup successful! Board " << m_selectedCardNo << " is ready for commands." << std::endl;
    m_successfullySetup = true;
    return true;
}

bool Rtc6Communicator::initializeAndShowBoardInfo() {
    std::cout << "\n[Rtc6Communicator] Initiating full board initialization and diagnostics..." << std::endl;

    if (connectAndSetupBoard()) {
        std::cout << "\n[Rtc6Communicator] Core setup complete. Retrieving board diagnostics..." << std::endl;

        printBoardVersions();
        printBoardSerialNumber();
        checkGlobalErrorStatus();

        std::cout << "\n[Rtc6Communicator] Board initialization and diagnostics complete." << std::endl;
        return true;
    }
    else {
        std::cerr << "\n[Rtc6Communicator] Core board setup failed. Cannot proceed with diagnostics." << std::endl;
        return false;
    }
}

void Rtc6Communicator::printBoardVersions() const {
    if (!m_isFirmwareLoaded) { // Crucial check: firmware must be loaded to get RTC version.
        std::cerr << "ERROR: [Rtc6Communicator] Cannot get board versions: Firmware not loaded or board not selected." << std::endl;
        return;
    }
    std::cout << "\n[Rtc6Communicator] Board " << m_selectedCardNo << " Firmware and BIOS Versions:" << std::endl;

    UINT rtcVersion = get_rtc_version(); // Operates on the currently selected card
    if (!checkError("get_rtc_version")) { // checkError will call n_get_last_error for m_selectedCardNo
        // Error already printed by checkError
    }
    else {
        std::cout << "  RTC Firmware Version: " << rtcVersion << std::endl;
    }

    UINT biosVersion = get_bios_version(); // Operates on the currently selected card
    if (!checkError("get_bios_version")) {
        // Error already printed
    }
    else {
        std::cout << "  BIOS Version: " << biosVersion << std::endl;
    }
}

void Rtc6Communicator::printBoardSerialNumber() const {
    if (!m_isBoardSelected) { // Need a selected card to get its serial number
        std::cerr << "ERROR: [Rtc6Communicator] Cannot get serial number: No board selected." << std::endl;
        return;
    }
    std::cout << "\n[Rtc6Communicator] Board " << m_selectedCardNo << " Serial Number:" << std::endl;

    // n_get_serial_number takes the specific card number (m_selectedCardNo).
    UINT serialNumber = n_get_serial_number(m_selectedCardNo);
    // For n_ functions, the error is specific to that card and command.
    // We check n_get_last_error for the card m_selectedCardNo.
    if (!checkError("n_get_serial_number (Card " + std::to_string(m_selectedCardNo) + ")")) {
        // Error already printed
    }
    else {
        // Check if serial number is 0, which might indicate an issue or an unprogrammed/simulated card
        if (serialNumber == 0) {
            std::cout << "  Serial Number: " << serialNumber << " (Note: 0 may indicate an issue or unprogrammed card)" << std::endl;
        }
        else {
            std::cout << "  Serial Number: " << serialNumber << std::endl;
        }
    }
}

void Rtc6Communicator::checkGlobalErrorStatus() const {
    if (!m_isBoardSelected && !m_isDllInitialized) {
        std::cerr << "[Rtc6Communicator] Cannot check global error status: DLL not even initialized." << std::endl;
        return;
    }

    // get_error() returns the 32-bit error status word for the *currently selected card*.
    // If no card is selected, n_get_error(0) would be for global errors.
    // We'll use m_selectedCardNo; if it's 0 (due to selection failure), get_error() might still report global issues.
    UINT32 accumulatedError = get_error(); // Uses currently selected card context (m_selectedCardNo)

    std::cout << "\n[Rtc6Communicator] Checking accumulated RTC6 status/errors for board context " << m_selectedCardNo << ":" << std::endl;
    if (accumulatedError != 0) {
        std::cerr << "  WARNING: Accumulated RTC6 errors/status flags active!" << std::endl;
        std::cerr << "  Error Code Mask (32-bit): " << std::bitset<32>(accumulatedError) << std::endl;
        // Refer to RTC6 Software Manual, section 6.8 "Error Word" for bit meanings. Example bits:
        if (accumulatedError & 0x00000001) std::cerr << "    - Bit 0 (BUSY): RTC6 is busy." << std::endl;
        if (accumulatedError & 0x00000002) std::cerr << "    - Bit 1 (PLL_NOT_LOCKED): HP option's PLL not locked." << std::endl;
        if (accumulatedError & 0x00000004) std::cerr << "    - Bit 2 (DSP_ERROR): DSP error." << std::endl;
        if (accumulatedError & 0x00000008) std::cerr << "    - Bit 3 (NO_PROG): No program loaded on RTC6." << std::endl;
        if (accumulatedError & 0x00000010) std::cerr << "    - Bit 4 (TEMP_OVER): Temperature too high." << std::endl;
        if (accumulatedError & 0x00000020) std::cerr << "    - Bit 5 (SPI_ERROR): Memory access error (SPI)." << std::endl;
        // ... and so on for other bits as per the manual.
    }
    else {
        std::cout << "  No accumulated errors or significant status flags active." << std::endl;
    }
}

bool Rtc6Communicator::isSuccessfullySetup() const {
    return m_successfullySetup;
}