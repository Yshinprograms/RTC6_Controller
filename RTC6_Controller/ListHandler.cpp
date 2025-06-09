#include "ListHandler.h"
#include "Rtc6Constants.h"
#include <iostream>
#include <cmath> // For std::round

ListHandler::ListHandler(InterfaceCommunicator& communicator)
    : m_communicator(communicator),
    m_currentListIdForFilling(1),
    m_currentListIdForExecution(0) {
    std::cout << "[ListHandler] Instance created. Default fill target: List 1." << std::endl;
}

// --- Workflow Management ---

bool ListHandler::setupAutoChangeMode() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot setup auto-change, Rtc6Communicator not ready." << std::endl;
        return false;
    }
    std::cout << "[ListHandler] Arming initial auto-change." << std::endl;
    auto_change(); // This is a one-time trigger for the next set_end_of_list
    return true;
}

void ListHandler::reArmAutoChange() {
    if (m_communicator.isSuccessfullySetup()) {
        std::cout << "[ListHandler] Re-arming auto-change for the next list transition." << std::endl;
        auto_change();
    }
}

bool ListHandler::beginListPreparation() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot begin list preparation, Rtc6Communicator not ready." << std::endl;
        return false;
    }
    std::cout << "[ListHandler] Beginning preparation for List " << m_currentListIdForFilling << std::endl;
    set_start_list(m_currentListIdForFilling);
    return true;
}

void ListHandler::endListPreparation() {
    std::cout << "[ListHandler] Ending preparation for List " << m_currentListIdForFilling << std::endl;
    set_end_of_list();
}

bool ListHandler::executeCurrentListAndCycle() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot execute list, Rtc6Communicator not ready." << std::endl;
        return false;
    }

    UINT listToExecute = m_currentListIdForFilling;
    std::cout << "[ListHandler] Commanding execution of List " << listToExecute << std::endl;
    execute_list(listToExecute);

    m_currentListIdForExecution = listToExecute;
    switchFillListTarget();

    return true;
}

// --- Status & State ---

bool ListHandler::isListBusy(UINT listIdToCheck) const {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "WARNING: [ListHandler] Communicator not ready; assuming list is busy." << std::endl;
        return true;
    }
    if (listIdToCheck != 1 && listIdToCheck != 2) {
        std::cerr << "WARNING: [ListHandler] Invalid listId " << listIdToCheck << " for isListBusy check." << std::endl;
        return true;
    }

    UINT status_word = read_status();

    if (listIdToCheck == 1) {
        return (status_word & Rtc6Constants::Status::BUSY1);
    }
    else { // listIdToCheck == 2
        return (status_word & Rtc6Constants::Status::BUSY2);
    }
}

UINT ListHandler::getCurrentFillListId() const {
    return m_currentListIdForFilling;
}

// --- List Command Abstractions ---

void ListHandler::addJumpAbsolute(INT x, INT y) {
    jump_abs(x, y);
}

void ListHandler::addMarkAbsolute(INT x, INT y) {
    mark_abs(x, y);
}

void ListHandler::addSetFocusOffset(INT offset_bits) {
    set_defocus_list(offset_bits);
}

void ListHandler::addSetMarkSpeed(double speed_mm_s) {
    // The API's set_mark_speed takes bits/ms.
    // Conversion: (mm/s) * (bits/mm) / (1000 ms/s) = bits/ms
    static constexpr double BITS_PER_MM = 1000.0; // Same as in GeometryHandler
    double speed_bits_per_ms = speed_mm_s * BITS_PER_MM / 1000.0;

    std::cout << "[ListHandler] Adding set_mark_speed: " << speed_bits_per_ms << " bits/ms (" << speed_mm_s << " mm/s)" << std::endl;
    set_mark_speed(speed_bits_per_ms);
}

void ListHandler::addSetLaserPower(UINT port, UINT power) {
    set_laser_power(port, power);
}

// --- Private Methods ---

void ListHandler::switchFillListTarget() {
    m_currentListIdForFilling = (m_currentListIdForFilling == 1) ? 2 : 1;
    std::cout << "[ListHandler] Switched internal fill target. Next list to fill: List " << m_currentListIdForFilling << std::endl;
}

int ListHandler::mmToBits(double mm) const {
    static constexpr double BITS_PER_MM = 1000.0; // Ensure this is consistent
    return static_cast<int>(std::round(mm * BITS_PER_MM));
}