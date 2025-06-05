// Interfaces/IRTCAPIDriver.h
#ifndef I_RTC_API_DRIVER_H
#define I_RTC_API_DRIVER_H

#include "Rtc6Common.h"
#include <string>

// Interface for low-level RTC6 driver operations.
// This abstract class defines the contract for any concrete RTC6 DLL wrapper.
// Methods here directly map to the core RTC6 DLL functions.
class IRTCAPIDriver {
public:
    virtual ~IRTCAPIDriver() = default;

    // --- Core Initialization & Board Management (often return UINT for error code) ---
    virtual UINT init_rtc6_dll() = 0;
    virtual UINT rtc6_count_cards() = 0;
    virtual UINT select_rtc(UINT cardNo) = 0;
    virtual UINT load_program_file(const char* path) = 0;
    virtual void free_rtc6_dll() = 0; // Returns void, error is handled by driver

    // --- Status & Version Queries ---
    virtual UINT get_rtc_version() = 0;
    virtual UINT get_bios_version() = 0;
    virtual UINT n_get_serial_number(UINT cardNo) = 0; // Use n_ version for clarity with cardNo
    virtual UINT n_get_last_error(UINT cardNo) = 0;
    virtual UINT get_error() = 0; // Accumulated error for current board
    virtual void reset_error(UINT cardNo, UINT errorMask = 0xFFFFFFFF) = 0;

    // --- General Configuration (Control Commands - immediate effect) ---
    virtual void set_laser_mode(UINT mode) = 0;
    virtual void set_laser_control(UINT ctrl) = 0; // Enables/disables laser signals, sets polarity
    virtual void set_mark_speed(double speed) = 0; // Speed in bits/ms
    virtual void set_jump_speed(double speed) = 0; // Speed in bits/ms

    // --- List Command Building (These commands are added to the list memory) ---
    // Note: The RTC6 API functions like jump_abs, mark_abs, micro_vector_abs are direct list commands.
    // They are called *within* the list building process.
    virtual void set_start_list(UINT listNo) = 0;
    virtual void set_end_of_list() = 0;
    virtual void execute_list(UINT listNo) = 0;

    virtual void jump_abs(LONG x, LONG y) = 0; // Standard jump command
    virtual void mark_abs(LONG x, LONG y) = 0; // Standard mark command

    // **Crucial for Micro-Vectors:** Laser state is directly controlled by micro_vector_abs/rel
    // The LasOn and LasOff parameters are bit masks for laser control signals (LASER1, LASER2, LASERON)
    // See RTC6 Manual 8.8 "micro_vector[*] Commands"
    virtual void micro_vector_abs(LONG x, LONG y, LONG laserOnBits, LONG laserOffBits) = 0;

    // --- Dynamic List Parameter Control (to be embedded in the list) ---
    virtual void set_laser_power_list(UINT powerValue) = 0; // For DAC output like analog power
    virtual void set_mark_speed_list(double speedValue) = 0;
    virtual void set_defocus_list(LONG zAxisValue) = 0; // For Z-axis control

    // --- List Status / Execution Status ---
    virtual UINT get_status() = 0; // General status, includes BUSY bit
    virtual UINT get_list_pointer() = 0; // Current instruction pointer in the list

    // Add more raw RTC6 DLL function declarations here as virtual = 0 as needed by other layers.
    // Ensure naming consistency (e.g., using snake_case like the original API).
};

#endif // I_RTC_API_DRIVER_H