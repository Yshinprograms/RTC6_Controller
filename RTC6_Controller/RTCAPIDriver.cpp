// Implementations/RTCAPIDriver.cpp
#include "RTCAPIDriver.h"
#include <bitset> // For debugging output of error codes as binary
#include "RTC6impl.h" // Include the actual RTC6 DLL API declarations

// --- Private Helper Method Implementation ---
// Centralizes error checking logic for all raw DLL calls.
void RTCAPIDriver::CheckAndThrowError(const std::string& commandName, UINT errorCodeToCheck) {
    if (errorCodeToCheck != 0) {
        std::stringstream ss;
        ss << "RTCAPIDriver Error during '" << commandName <<
            "' on card " << cardNo_ << ". Raw Error Code: " << errorCodeToCheck <<
            " (0x" << std::hex << errorCodeToCheck << std::dec << " | " << std::bitset<32>(errorCodeToCheck) << ")";
        throw Rtc6Exception(ss.str(), errorCodeToCheck);
    }
}

// --- Constructor & Destructor ---
RTCAPIDriver::RTCAPIDriver(UINT cardNo)
    : cardNo_(cardNo), isDllInitialized_(false) {
    std::cout << "[RTCAPIDriver] Instance created for card: " << cardNo_ << std::endl;
}

RTCAPIDriver::~RTCAPIDriver() {
    // Only free DLL resources if this instance was responsible for initializing them.
    if (isDllInitialized_) {
        std::cout << "[RTCAPIDriver] Automatically freeing RTC6 DLL resources (called by this instance)." << std::endl;
        ::free_rtc6_dll(); // Call the global DLL function
        isDllInitialized_ = false;
    }
}

// --- Implementations of IRTCAPIDriver Methods (RAW DLL Calls) ---

// Initializes the RTC6 DLL. This MUST be the first RTC6 API call.
UINT RTCAPIDriver::init_rtc6_dll() {
    std::cout << "[RTCAPIDriver] Calling init_rtc6_dll()..." << std::endl;
    UINT errorCode = ::init_rtc6_dll(); // Call the global DLL function (note '::' for global scope)
    CheckAndThrowError("init_rtc6_dll", errorCode);
    if (errorCode == 0) { // If successful, mark that this instance initialized it
        isDllInitialized_ = true;
    }
    return errorCode;
}

// Returns the number of detected RTC6 boards.
UINT RTCAPIDriver::rtc6_count_cards() {
    UINT count = ::rtc6_count_cards();
    // rtc6_count_cards itself doesn't return an error code directly,
    // so no CheckAndThrowError here. Errors would be caught by subsequent calls.
    return count;
}

// Selects the active RTC6 board for subsequent single-board commands.
UINT RTCAPIDriver::select_rtc(UINT cardNo) {
    std::cout << "[RTCAPIDriver] Calling select_rtc(" << cardNo << ")..." << std::endl;
    UINT selected = ::select_rtc(cardNo); // Call the global DLL function
    // select_rtc returns 0 on failure, so we must check n_get_last_error specifically for the card.
    CheckAndThrowError("select_rtc", ::n_get_last_error(cardNo));
    cardNo_ = selected; // Update internal state with the successfully selected card
    return selected;
}

// Loads the DSP firmware onto the RTC6 board.
UINT RTCAPIDriver::load_program_file(const char* path) {
    std::cout << "[RTCAPIDriver] Calling load_program_file(" << (path ? path : "0") << ")..." << std::endl;
    UINT errorCode = ::load_program_file(path); // Call the global DLL function
    CheckAndThrowError("load_program_file", errorCode);
    return errorCode;
}

// Releases RTC6 DLL resources.
void RTCAPIDriver::free_rtc6_dll() {
    // Only free if previously initialized by this instance
    if (isDllInitialized_) {
        std::cout << "[RTCAPIDriver] Calling free_rtc6_dll()..." << std::endl;
        ::free_rtc6_dll(); // Call the global DLL function
        isDllInitialized_ = false; // Mark as no longer initialized by this instance
    }
}

// Retrieves RTC firmware version.
UINT RTCAPIDriver::get_rtc_version() {
    UINT version = ::get_rtc_version(); // Call the global DLL function
    CheckAndThrowError("get_rtc_version", ::n_get_last_error(cardNo_)); // Check error for *this* card
    return version;
}

// Retrieves board BIOS version.
UINT RTCAPIDriver::get_bios_version() {
    UINT version = ::get_bios_version(); // Call the global DLL function
    CheckAndThrowError("get_bios_version", ::n_get_last_error(cardNo_));
    return version;
}

// Retrieves unique board serial number for the specified card.
UINT RTCAPIDriver::n_get_serial_number(UINT cardNo) {
    UINT serialNumber = ::n_get_serial_number(cardNo); // Call the multi-board DLL function
    CheckAndThrowError("n_get_serial_number", ::n_get_last_error(cardNo));
    return serialNumber;
}

// Retrieves the last error code for a specific card.
UINT RTCAPIDriver::n_get_last_error(UINT cardNo) {
    return ::n_get_last_error(cardNo); // This function is inherently for checking errors, doesn't throw on its own result
}

// Retrieves accumulated error bits for the currently selected board.
UINT RTCAPIDriver::get_error() {
    return ::get_error(); // Call the global DLL function (defaults to selected card)
}

// Resets error bits for a specific card.
void RTCAPIDriver::reset_error(UINT cardNo, UINT errorMask) {
    ::n_reset_error(cardNo, errorMask); // Use n_ version for specific card
    // reset_error typically doesn't return an error code itself.
}

// --- General Configuration (Control Commands) ---
void RTCAPIDriver::set_laser_mode(UINT mode) {
    ::set_laser_mode(mode);
    CheckAndThrowError("set_laser_mode", ::n_get_last_error(cardNo_));
}

void RTCAPIDriver::set_laser_control(UINT ctrl) {
    ::set_laser_control(ctrl);
    CheckAndThrowError("set_laser_control", ::n_get_last_error(cardNo_));
}

void RTCAPIDriver::set_mark_speed(double speed) {
    ::set_mark_speed(speed);
    CheckAndThrowError("set_mark_speed", ::n_get_last_error(cardNo_));
}

void RTCAPIDriver::set_jump_speed(double speed) {
    ::set_jump_speed(speed);
    CheckAndThrowError("set_jump_speed", ::n_get_last_error(cardNo_));
}

// --- List Command Building ---
void RTCAPIDriver::set_start_list(UINT listNo) {
    ::set_start_list(listNo);
    CheckAndThrowError("set_start_list", ::n_get_last_error(cardNo_));
}

void RTCAPIDriver::set_end_of_list() {
    ::set_end_of_list();
    CheckAndThrowError("set_end_of_list", ::n_get_last_error(cardNo_));
}

void RTCAPIDriver::execute_list(UINT listNo) {
    ::execute_list(listNo);
    CheckAndThrowError("execute_list", ::n_get_last_error(cardNo_));
}

void RTCAPIDriver::jump_abs(LONG x, LONG y) {
    ::jump_abs(x, y);
    CheckAndThrowError("jump_abs", ::n_get_last_error(cardNo_));
}

void RTCAPIDriver::mark_abs(LONG x, LONG y) {
    ::mark_abs(x, y);
    CheckAndThrowError("mark_abs", ::n_get_last_error(cardNo_));
}

// **Micro-Vector Specific API Call**
void RTCAPIDriver::micro_vector_abs(LONG x, LONG y, LONG laserOnBits, LONG laserOffBits) {
    // This is the actual RTC6 API call for micro-vectors.
    // It takes absolute destination (X,Y) and laser control bits for that 10us segment.
    ::micro_vector_abs(x, y, laserOnBits, laserOffBits);
    CheckAndThrowError("micro_vector_abs", ::n_get_last_error(cardNo_));
}

// --- Dynamic List Parameter Control ---
void RTCAPIDriver::set_laser_power_list(UINT powerValue) {
    ::set_laser_power_list(powerValue);
    CheckAndThrowError("set_laser_power_list", ::n_get_last_error(cardNo_));
}

void RTCAPIDriver::set_mark_speed_list(double speedValue) {
    ::set_mark_speed_list(speedValue);
    CheckAndThrowError("set_mark_speed_list", ::n_get_last_error(cardNo_));
}

void RTCAPIDriver::set_defocus_list(LONG zAxisValue) {
    ::set_defocus_list(zAxisValue);
    CheckAndThrowError("set_defocus_list", ::n_get_last_error(cardNo_));
}

// --- List Status / Execution Status ---
UINT RTCAPIDriver::get_status() {
    return ::get_status();
}

UINT RTCAPIDriver::get_list_pointer() {
    return ::get_list_pointer();
}