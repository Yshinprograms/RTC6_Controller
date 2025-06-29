#include "ListHandler.h"
#include "Rtc6Constants.h"
#include <iostream>
#include <iomanip> // Required for std::hex/dec formatting
#include <cmath>

// ListHandler orchestrates the creation and execution of command lists for the RTC6 board.
// It requires a communicator to check board readiness and an RTC API wrapper to send commands.
ListHandler::ListHandler(InterfaceCommunicator& communicator, InterfaceRtcApi& rtcApi)
    : m_communicator(communicator),
    m_rtcApi(rtcApi),
    m_currentListIdForFilling(1), // Start by preparing commands for List 1.
    m_currentListIdForExecution(0),  // No list is executing initially.
	m_lastExecutedListId(0) // Initialize last executed list ID to 0.
{
    std::cout << "[ListHandler] Instance created. Default fill target: List 1." << std::endl;
}

ListHandler::~ListHandler() {}

UINT ListHandler::getLastExecutedListId() const {
    return m_lastExecutedListId;
}

// Arms the RTC6's automatic list-switching capability for the first time.
bool ListHandler::setupAutoChangeMode() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot setup auto-change, Rtc6Communicator not ready." << std::endl;
        return false;
    }
    std::cout << "[ListHandler] Arming initial auto-change." << std::endl;
    std::cout << "  [API CALL] api_auto_change()" << std::endl;
    m_rtcApi.api_auto_change();
    return true;
}

// Re-arms the RTC6's automatic list-switching capability for subsequent transitions.
void ListHandler::reArmAutoChange() {
    if (m_communicator.isSuccessfullySetup()) {
        std::cout << "[ListHandler] Re-arming auto-change for the next list transition." << std::endl;
        std::cout << "  [API CALL] api_auto_change()" << std::endl;
        m_rtcApi.api_auto_change();
    }
}

// Designates the start of a command list buffer in the RTC6's memory.
bool ListHandler::beginListPreparation() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot begin list preparation, Rtc6Communicator not ready." << std::endl;
        return false;
    }
    std::cout << "[ListHandler] Beginning preparation for List " << m_currentListIdForFilling << std::endl;
    std::cout << "  [API CALL] api_set_start_list(list_id=" << m_currentListIdForFilling << ")" << std::endl;
    m_rtcApi.api_set_start_list(m_currentListIdForFilling);
    return true;
}

// Designates the end of a command list buffer.
void ListHandler::endListPreparation() {
    std::cout << "[ListHandler] Ending preparation for List " << m_currentListIdForFilling << std::endl;
    std::cout << "  [API CALL] api_set_end_of_list()" << std::endl;
    m_rtcApi.api_set_end_of_list();
}

// Triggers the execution of the list that was just prepared.
bool ListHandler::executeCurrentListAndCycle() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot execute list, Rtc6Communicator not ready." << std::endl;
        return false;
    }

    UINT listToExecute = m_currentListIdForFilling;
    std::cout << "[ListHandler] Commanding execution of List " << listToExecute << std::endl;
    std::cout << "  [API CALL] api_execute_list(list_id=" << listToExecute << ")" << std::endl;
    m_rtcApi.api_execute_list(listToExecute);
    m_lastExecutedListId = m_currentListIdForFilling;

    m_currentListIdForExecution = listToExecute;
    switchFillListTarget();
    return true;
}

// Checks the RTC6 status bits to see if a specific list is still being processed.
bool ListHandler::isListBusy(UINT listIdToCheck) const {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "WARNING: [ListHandler] Communicator not ready; assuming list is busy." << std::endl;
        return true;
    }
    if (listIdToCheck != 1 && listIdToCheck != 2) {
        std::cerr << "WARNING: [ListHandler] Invalid listId " << listIdToCheck << " for isListBusy check." << std::endl;
        return true;
    }

    // NOTE: This function is often called in a tight loop. Logging every call can spam the console.
    // However, for debugging, it's useful to see the raw value being returned.
    UINT status_word = m_rtcApi.api_read_status();

    if (listIdToCheck == 1) {
        return (status_word & Rtc6Constants::Status::BUSY1);
    }
    else { // listIdToCheck is 2
        return (status_word & Rtc6Constants::Status::BUSY2);
    }
}

UINT ListHandler::getCurrentFillListId() const {
    return m_currentListIdForFilling;
}

// --- List Command Functions ---

void ListHandler::addJumpAbsolute(INT x, INT y) {
    std::cout << "  [API CALL] api_jump_abs(x=" << x << ", y=" << y << ")" << std::endl;
    m_rtcApi.api_jump_abs(x, y);
}

void ListHandler::addMarkAbsolute(INT x, INT y) {
    std::cout << "  [API CALL] api_mark_abs(x=" << x << ", y=" << y << ")" << std::endl;
    m_rtcApi.api_mark_abs(x, y);
}

void ListHandler::addSetFocusOffset(INT offset_bits) {
    std::cout << "  [API CALL] api_set_defocus_list(offset=" << offset_bits << ")" << std::endl;
    m_rtcApi.api_set_defocus_list(offset_bits);
}

void ListHandler::addSetMarkSpeed(double speed_mm_s) {
    static constexpr double BITS_PER_MM = 1000.0;
    double speed_bits_per_ms = speed_mm_s * BITS_PER_MM / 1000.0;

    std::cout << "[ListHandler] Adding set_mark_speed: " << speed_bits_per_ms << " bits/ms (" << speed_mm_s << " mm/s)" << std::endl;
    std::cout << "  [API CALL] api_set_mark_speed(speed=" << speed_bits_per_ms << ")" << std::endl;
    m_rtcApi.api_set_mark_speed(speed_bits_per_ms);
}

void ListHandler::addSetLaserPower(UINT port, UINT power) {
    std::cout << "  [API CALL] api_set_laser_power(port=" << port << ", power=" << power << ")" << std::endl;
    m_rtcApi.api_set_laser_power(port, power);
}

// Implements the core "ping-pong" logic by flipping the target buffer.
void ListHandler::switchFillListTarget() {
    m_currentListIdForFilling = (m_currentListIdForFilling == 1) ? 2 : 1;
    std::cout << "[ListHandler] Switched internal fill target. Next list to fill: List " << m_currentListIdForFilling << std::endl;
}

// Private helper to convert from physical units (mm) to hardware units (bits).
int ListHandler::mmToBits(double mm) const {
    static constexpr double BITS_PER_MM = 1000.0;
    return static_cast<int>(std::round(mm * BITS_PER_MM));
}