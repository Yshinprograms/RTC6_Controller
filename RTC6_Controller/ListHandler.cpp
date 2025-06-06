// ListHandler.cpp
#include "ListHandler.h"
#include <iostream> // For basic diagnostic messages

ListHandler::ListHandler(Rtc6Communicator& communicator)
    : m_communicator(communicator),
    m_currentListIdForFilling(1),   // Default to start filling List 1
    m_currentListIdForExecution(0), // Initially, no list has been commanded to execute
    m_autoChangeEnabled(false),
    m_firstListExecuted(false) {
    std::cout << "[ListHandler] Instance created. Default fill target: List 1." << std::endl;
}

bool ListHandler::setupAutoChangeMode() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot setup auto-change, Rtc6Communicator not ready." << std::endl;
        return false;
    }
    std::cout << "[ListHandler] Enabling auto-change mode via RTC6 API." << std::endl;
    auto_change(); // RTC6 API call - assumes it doesn't return an error or communicator handles it
    m_autoChangeEnabled = true;
    m_currentListIdForFilling = 1; // With auto-change, typically start filling List 1
    m_currentListIdForExecution = 2; // And assume List 2 was the "other" one initially (or will be)
    // This might need refinement based on exact auto_change startup.
    // More robustly, after first execute_list(1), exec becomes 1, fill becomes 2.
    m_firstListExecuted = false;   // Reset for auto-change logic
    return true;
}

// Simplified: always uses the current internal fill target m_currentListIdForFilling
bool ListHandler::beginListPreparation() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot begin list preparation, Rtc6Communicator not ready." << std::endl;
        return false;
    }
    // Optional: More robust check if the target fill list is actually free
    // if (m_autoChangeEnabled && isListBusy(m_currentListIdForFilling)) {
    //    std::cerr << "ERROR: [ListHandler] Target fill list " << m_currentListIdForFilling << " is unexpectedly busy." << std::endl;
    //    return false;
    // }

    std::cout << "[ListHandler] Beginning preparation for List " << m_currentListIdForFilling << std::endl;
    set_start_list(m_currentListIdForFilling); // RTC6 API call
    // No change to m_currentListIdForExecution here; that's set upon execution.
    return true;
}

/*
// Alternative implementation if you want to explicitly pass the list ID
bool ListHandler::beginListPreparation(UINT listId) {
    if (listId != 1 && listId != 2) {
        std::cerr << "ERROR: [ListHandler] Invalid list ID for preparation: " << listId << std::endl;
        return false;
    }
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot begin list prep, communicator not ready." << std::endl;
        return false;
    }
    std::cout << "[ListHandler] Beginning preparation for List " << listId << std::endl;
    set_start_list(listId); // RTC6 API call
    m_currentListIdForFilling = listId; // Set the current fill target
    return true;
}
*/

void ListHandler::addLaserSignalOn() {
    laser_signal_on_list(); // RTC6 API call
}

void ListHandler::addLaserSignalOff() {
    laser_signal_off_list(); // RTC6 API call
}

void ListHandler::addSetLaserPower(UINT power) {
    set_laser_power(power); // RTC6 API call
}

void ListHandler::addSetZPosition(INT zPos) {
    set_z_pos_list(zPos); // RTC6 API call
}

void ListHandler::addJumpAbsolute(INT x, INT y, INT z) {
    if (z == 0 && m_communicator.isSuccessfullySetup()) { // Assuming 2D jump if z is default/0
        jump_abs(x, y);
    }
    else {
        // jump_abs_3d_list(x, y, z); // If you have a 3D jump variant
        // For now, stick to 2D or make z-setting separate via addSetZPosition
        jump_abs(x, y); // If only 2D jumps are directly supported by this simple handler
        if (z != 0) addSetZPosition(z); // And then set Z if needed
    }
}


void ListHandler::addMicroVectorAbs(INT x_target, INT y_target, UINT dt) {
    // As discussed, the exact meaning of 'x_target, y_target' for micro_vector_abs
    // (absolute end-point vs relative displacement) is crucial and depends on RTC6 API.
    // Assuming it takes absolute target for the micro-segment's end for now.
    micro_vector_abs(x_target, y_target, dt); // RTC6 API call
}

// void ListHandler::addMicroVectorRel(INT dx, INT dy, UINT dt) {
//     micro_vector_rel(dx, dy, dt); // RTC6 API call
// }

void ListHandler::endListPreparation() {
    // m_currentListIdForFilling should already be set by beginListPreparation()
    std::cout << "[ListHandler] Ending preparation for List " << m_currentListIdForFilling << std::endl;
    set_end_of_list(); // RTC6 API call
}

bool ListHandler::executeCurrentListAndCycle() {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "ERROR: [ListHandler] Cannot execute list, Rtc6Communicator not ready." << std::endl;
        return false;
    }

    UINT listToExecute = m_currentListIdForFilling; // The list just prepared is the one to execute
    std::cout << "[ListHandler] Commanding execution of List " << listToExecute << std::endl;

    execute_list(listToExecute); // RTC6 API call
    m_currentListIdForExecution = listToExecute; // This list is now the (last) one commanded to execute

    if (m_autoChangeEnabled) {
        if (!m_firstListExecuted) {
            m_firstListExecuted = true;
        }
        switchFillListTarget(); // Prepare to fill the other list next
    }
    else {
        // In manual mode, the user explicitly calls beginListPreparation for the desired list.
        // We might still switch the internal default target for convenience if desired.
        switchFillListTarget();
    }
    return true;
}


bool ListHandler::isListBusy(UINT listIdToCheck) const {
    if (!m_communicator.isSuccessfullySetup()) {
        std::cerr << "WARNING: [ListHandler] Communicator not ready, assuming list is busy for isListBusy check." << std::endl;
        return true; // Fail safe
    }
    if (listIdToCheck != 1 && listIdToCheck != 2) {
        std::cerr << "WARNING: [ListHandler] Invalid listId " << listIdToCheck << " for isListBusy check." << std::endl;
        return true;
    }


    UINT rtc_status_word = get_status(); // Global status
    // m_communicator.checkError("get_status"); // Check for errors from get_status itself if your comm layer supports it

    bool isBoardGloballyBusy = (rtc_status_word & 0x00000001); // Bit #0: BUSY list execution status

    if (!isBoardGloballyBusy) {
        return false; // If board is not busy, the specific listIdToCheck is definitely not busy.
    }

    // Board is globally busy. Now, is it busy with listIdToCheck?
    // m_currentListIdForExecution holds the ID of the list that was LAST COMMANDED to execute.
    // If auto_change is on, the board might have already switched to the *other* list
    // if m_currentListIdForExecution finished very quickly.
    // This makes a precise "is list X busy?" hard with only get_status().
    // read_status() is needed for definitive status of List1 vs List2.

    // Simplification: If the board is busy, we assume the list we *think* should be running
    // (m_currentListIdForExecution) is the one causing the business.
    // If listIdToCheck is this list, then it's busy.
    // If listIdToCheck is NOT this list, then the *other* list is busy, so listIdToCheck is free.
    if (listIdToCheck == m_currentListIdForExecution) {
        return true; // Board is busy, and it's with the list we last told it to run.
    }
    else {
        // Board is busy, but with the *other* list. So, listIdToCheck is free for filling.
        return false;
    }
}

UINT ListHandler::getCurrentFillListId() const {
    return m_currentListIdForFilling;
}

void ListHandler::switchFillListTarget() {
    if (m_currentListIdForFilling == 1) {
        m_currentListIdForFilling = 2;
    }
    else {
        m_currentListIdForFilling = 1;
    }
    // m_currentListIdForExecution is NOT changed here. It's changed when execute_list is called.
    std::cout << "[ListHandler] Switched internal fill target. Next list to fill: List " << m_currentListIdForFilling << std::endl;
}