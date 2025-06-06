#include "ListHandler.h"
#include "Rtc6Constants.h" // Include the new constants file
#include <iostream>

ListHandler::ListHandler(Rtc6Communicator& communicator)
    : m_communicator(communicator),
    m_currentListIdForFilling(1),
    m_currentListIdForExecution(0), // No list is executing initially
    m_autoChangeEnabled(false),
    m_firstListExecuted(false) {
    std::cout << "[ListHandler] Instance created. Default fill target: List 1." << std::endl;
}

bool ListHandler::setupAutoChangeMode() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot setup auto-change, Rtc6Communicator not ready." << std::endl;
        return false;
    }
    // Note: auto_change() is a one-time trigger for the *next* end-of-list event.
    // It does not set a permanent "mode" on the board; our class state handles the logic.
    std::cout << "[ListHandler] Setting up auto-change mode." << std::endl;
    auto_change();
    m_autoChangeEnabled = true;
    m_currentListIdForFilling = 1;
    return true;
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

void ListHandler::addLaserSignalOn() {
    laser_signal_on_list();
}

void ListHandler::addLaserSignalOff() {
    laser_signal_off_list();
}

void ListHandler::addSetLaserPower(UINT port, UINT power) {
    set_laser_power(port, power);
}

void ListHandler::addSetZPosition(INT zPos) {
    // Correct API call for setting Z-focus shift in a list.
    set_defocus_list(zPos);
}

void ListHandler::addJumpAbsolute(INT x, INT y) {
    jump_abs(x, y);
}

void ListHandler::addMicroVectorAbs(INT x_target, INT y_target, INT lasOnDelay, INT lasOffDelay) {
    // Correct API call with laser on/off delays.
    micro_vector_abs(x_target, y_target, lasOnDelay, lasOffDelay);
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

    // Switch the target for the *next* fill operation.
    switchFillListTarget();

    return true;
}

bool ListHandler::isListBusy(UINT listIdToCheck) const {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "WARNING: [ListHandler] Communicator not ready; assuming list is busy." << std::endl;
        return true;
    }
    if (listIdToCheck != 1 && listIdToCheck != 2) {
        std::cerr << "WARNING: [ListHandler] Invalid listId " << listIdToCheck << " for isListBusy check." << std::endl;
        return true;
    }

    // Use read_status() to check the BUSY flag for a *specific* list (BUSY1/BUSY2).
    // This is more accurate than the global get_status() for ping-pong buffering.
    UINT status_word = read_status();

    if (listIdToCheck == 1) {
        return (status_word & Rtc6Constants::Status::BUSY1); // Check BUSY1 flag (Bit 4)
    }
    else { // listIdToCheck == 2
        return (status_word & Rtc6Constants::Status::BUSY2); // Check BUSY2 flag (Bit 5)
    }
}

UINT ListHandler::getCurrentFillListId() const {
    return m_currentListIdForFilling;
}

void ListHandler::switchFillListTarget() {
    m_currentListIdForFilling = (m_currentListIdForFilling == 1) ? 2 : 1;
    std::cout << "[ListHandler] Switched internal fill target. Next list to fill: List " << m_currentListIdForFilling << std::endl;
}