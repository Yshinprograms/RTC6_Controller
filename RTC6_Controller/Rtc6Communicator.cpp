#include "Rtc6Communicator.h"
#include "Rtc6Constants.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <bitset>

Rtc6Communicator::Rtc6Communicator(UINT boardId)
    : m_boardId(boardId),
    m_selectedCardNo(0),
    m_isDllInitialized(false),
    m_isBoardSelected(false),
    m_isFirmwareLoaded(false),
    m_successfullySetup(false) {
    std::cout << "[Rtc6Communicator] Instance created for board ID: " << m_boardId << std::endl;
    std::cout << "[Rtc6Communicator] INFO: Ensure PC sleep/standby modes are DISABLED for RTC6 operation." << std::endl;
}

Rtc6Communicator::~Rtc6Communicator() {
    if (m_isDllInitialized) {
        std::cout << "[Rtc6Communicator] Releasing RTC6 DLL resources..." << std::endl;
        free_rtc6_dll();
    }
    std::cout << "[Rtc6Communicator] Instance destroyed." << std::endl;
}

bool Rtc6Communicator::checkError(const std::string& commandName, UINT rtcError) const {
    if (rtcError != 0) {
        std::cerr << "ERROR: [Rtc6Communicator] Command '" << commandName
            << "' failed on board context " << m_selectedCardNo
            << "." << std::endl;
        std::cerr << "       RTC6 Error Code: " << rtcError << " (Decimal), 0x"
            << std::hex << rtcError << std::dec << " (Hex)" << std::endl;
        return false;
    }
    return true;
}

bool Rtc6Communicator::checkError(const std::string& commandName) const {
    UINT cardContextForError = m_selectedCardNo;
    if (commandName == "init_rtc6_dll" || commandName == "rtc6_count_cards" || (commandName == "select_rtc" && m_selectedCardNo == 0)) {
        cardContextForError = 0;
    }
    else if (!m_isBoardSelected) {
        std::cerr << "WARNING: [Rtc6Communicator] Attempting to get error for '" << commandName
            << "' but no board is actively selected. Checking global error." << std::endl;
        cardContextForError = 0;
    }
    UINT lastError = n_get_last_error(cardContextForError);
    return checkError(commandName, lastError);
}

bool Rtc6Communicator::initializeDll() {
    std::cout << "\n[Rtc6Communicator] Initializing RTC6 DLL..." << std::endl;
    UINT errorCode = init_rtc6_dll();
    if (errorCode != 0) {
        checkError("init_rtc6_dll", errorCode);
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
    UINT count = rtc6_count_cards();
    std::cout << "\n[Rtc6Communicator] Detected " << count << " RTC6 board(s)." << std::endl;
    if (count == 0) {
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
    UINT actualSelectedCard = select_rtc(m_boardId);
    if (actualSelectedCard == 0) {
        std::cerr << "FATAL ERROR: [Rtc6Communicator] Failed to select RTC6 board " << m_boardId << "." << std::endl;
        UINT errorVal = n_get_last_error(0);
        if (errorVal == 0 && m_boardId != 0) {
            errorVal = n_get_last_error(m_boardId);
        }
        checkError("select_rtc", errorVal != 0 ? errorVal : 1);
        std::cerr << "       Check card number, physical connection, driver conflicts, or access rights (e.g., run as admin if needed)." << std::endl;
        m_selectedCardNo = 0;
        m_isBoardSelected = false;
        return false;
    }
    m_selectedCardNo = actualSelectedCard;
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
    UINT errorCode = load_program_file(nullptr);
    if (errorCode != 0) {
        checkError("load_program_file", errorCode);
        std::cerr << "       Ensure RTC6OUT.out, RTC6RBF.rbf, RTC6DAT.dat are in the executable directory," << std::endl;
        std::cerr << "       are not corrupted, and are compatible with the RTC6DLLx64.dll version." << std::endl;
        return false;
    }
    m_isFirmwareLoaded = true;
    std::cout << "[Rtc6Communicator] DSP program file loaded successfully onto board " << m_selectedCardNo << "." << std::endl;
    return true;
}

bool Rtc6Communicator::connectAndSetupBoard() {
    m_successfullySetup = false;
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
    if (!m_isFirmwareLoaded) {
        std::cerr << "ERROR: [Rtc6Communicator] Cannot get board versions: Firmware not loaded or board not selected." << std::endl;
        return;
    }
    std::cout << "\n[Rtc6Communicator] Board " << m_selectedCardNo << " Firmware and BIOS Versions:" << std::endl;
    UINT rtcVersion = get_rtc_version();
    if (!checkError("get_rtc_version")) {
    }
    else {
        std::cout << "  RTC Firmware Version: " << rtcVersion << std::endl;
    }
    UINT biosVersion = get_bios_version();
    if (!checkError("get_bios_version")) {
    }
    else {
        std::cout << "  BIOS Version: " << biosVersion << std::endl;
    }
}

void Rtc6Communicator::printBoardSerialNumber() const {
    if (!m_isBoardSelected) {
        std::cerr << "ERROR: [Rtc6Communicator] Cannot get serial number: No board selected." << std::endl;
        return;
    }
    std::cout << "\n[Rtc6Communicator] Board " << m_selectedCardNo << " Serial Number:" << std::endl;
    UINT serialNumber = n_get_serial_number(m_selectedCardNo);
    if (!checkError("n_get_serial_number (Card " + std::to_string(m_selectedCardNo) + ")")) {
    }
    else {
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

    // get_error() returns the ACCUMULATED error register, which persists until cleared.
    UINT32 accumulatedError = get_error();

    std::cout << "\n[Rtc6Communicator] Checking accumulated RTC6 errors for board context " << m_selectedCardNo << ":" << std::endl;

    if (accumulatedError != Rtc6Constants::Error::NONE) {
        std::cerr << "  WARNING: Accumulated RTC6 errors/status flags active!" << std::endl;
        std::cerr << "  Error Code Mask (32-bit): " << std::bitset<32>(accumulatedError) << std::endl;

        if (accumulatedError & Rtc6Constants::Error::TIMEOUT)         std::cerr << "    - Bit 3 (Timeout): No response from board. Check firmware load." << std::endl;
        if (accumulatedError & Rtc6Constants::Error::BUSY)            std::cerr << "    - Bit 5 (Busy): List processing is active." << std::endl;
        if (accumulatedError & Rtc6Constants::Error::REJECTED)        std::cerr << "    - Bit 6 (Rejected): List command rejected (e.g., invalid input pointer)." << std::endl;
        if (accumulatedError & Rtc6Constants::Error::VERSION_MISMATCH) std::cerr << "    - Bit 8 (VersionMismatch): Incompatible DLL, RBF, or OUT versions." << std::endl;
        if (accumulatedError & Rtc6Constants::Error::TYPE_REJECTED)    std::cerr << "    - Bit 10 (TypeRejected): Command sent to wrong board type." << std::endl;
        if (accumulatedError & Rtc6Constants::Error::ETH_ERROR)        std::cerr << "    - Bit 13 (EthError): General Ethernet error." << std::endl;
    }
    else {
        std::cout << "  No accumulated errors or significant status flags active." << std::endl;
    }
}


bool Rtc6Communicator::isSuccessfullySetup() const {
    return m_successfullySetup;
}