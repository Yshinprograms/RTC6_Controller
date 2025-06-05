// Implementations/RTCAPIDriver.h
#ifndef RTC_API_DRIVER_H
#define RTC_API_DRIVER_H

#include "IRTCAPIDriver.h" // Inherit from the interface
#include <iostream>                      // For internal logging/debug statements
#include <string>

// Concrete implementation of IRTCAPIDriver that directly calls the RTC6DLL.dll functions.
// This class is the direct "communication layer" with the hardware driver.
class RTCAPIDriver : public IRTCAPIDriver {
public:
    // Constructor: Initializes the driver, optionally for a specific card.
    explicit RTCAPIDriver(UINT cardNo = 1);

    // Destructor: Responsible for releasing DLL resources if this instance initialized them.
    virtual ~RTCAPIDriver();

    // Prevent copying and assignment (good practice for resource-managing classes)
    RTCAPIDriver(const RTCAPIDriver&) = delete;
    RTCAPIDriver& operator=(const RTCAPIDriver&) = delete;

    // --- Implementations of IRTCAPIDriver methods ---

    // Core Initialization & Board Management
    UINT init_rtc6_dll() override;
    UINT rtc6_count_cards() override;
    UINT select_rtc(UINT cardNo) override;
    UINT load_program_file(const char* path) override;
    void free_rtc6_dll() override;

    // Status & Version Queries
    UINT get_rtc_version() override;
    UINT get_bios_version() override;
    UINT n_get_serial_number(UINT cardNo) override;
    UINT n_get_last_error(UINT cardNo) override;
    UINT get_error() override;
    void reset_error(UINT cardNo, UINT errorMask = 0xFFFFFFFF) override;

    // General Configuration (Control Commands)
    void set_laser_mode(UINT mode) override;
    void set_laser_control(UINT ctrl) override;
    void set_mark_speed(double speed) override;
    void set_jump_speed(double speed) override;

    // List Command Building
    void set_start_list(UINT listNo) override;
    void set_end_of_list() override;
    void execute_list(UINT listNo) override;
    void jump_abs(LONG x, LONG y) override;
    void mark_abs(LONG x, LONG y) override;
    void micro_vector_abs(LONG x, LONG y, LONG laserOnBits, LONG laserOffBits) override;

    // Dynamic List Parameter Control
    void set_laser_power_list(UINT powerValue) override;
    void set_mark_speed_list(double speedValue) override;
    void set_defocus_list(LONG zAxisValue) override;

    // List Status / Execution Status
    UINT get_status() override;
    UINT get_list_pointer() override;

private:
    UINT cardNo_;              // The RTC6 card number this driver instance is associated with.
    bool isDllInitialized_;    // Flag to track if this instance successfully initialized the DLL.

    // Private helper method to check for RTC6 errors and throw Rtc6Exception.
    // All calls to global RTC6 DLL functions should be wrapped by this.
    void CheckAndThrowError(const std::string& commandName, UINT errorCodeToCheck);
};

#endif // RTC_API_DRIVER_H