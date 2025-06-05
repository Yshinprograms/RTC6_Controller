// Implementations/LaserControl.cpp
#include "LaserControl.h"

// Constructor: Initializes with the low-level RTC6 driver.
LaserControl::LaserControl(IRTCAPIDriver* driver)
    : driver_(driver) {
    if (driver_ == nullptr) {
        throw std::invalid_argument("LaserControl: IRTCAPIDriver pointer cannot be null.");
    }
}

// --- Implementations of ILaserControl methods ---

// Getters for List-based Parameter Values
// These return the values that MicroVectorProcessor will embed in list commands.
UINT LaserControl::GetPowerForList() const {
    return currentLaserPowerValue_;
}

double LaserControl::GetSpeedForList() const {
    return currentMarkSpeed_;
}

LONG LaserControl::GetZAxisHeightForList() const {
    return currentZAxisHeight_;
}

// Getters for Laser Control Bitmasks (for micro_vector_abs/rel)
// These define the laser ON/OFF state bits.
// LASER1 (Bit 0), LASER2 (Bit 1), LASERON (Bit 2) from set_laser_control.
// For micro_vector_abs, you specify a bitmask for LASER1, LASER2, LASERON.
// For example, if LASERON (Bit 2) is active-HIGH, then 0x4 (binary 100) turns LASERON ON.
// This example assumes LASERON (Bit 2) is active-LOW (as per 0x18 in RTC6 manual examples).
// So, LASERON ON = Bit 2 is LOW (0). LASERON OFF = Bit 2 is HIGH (1).
// This is a complex area, depends on `set_laser_control` settings.
// For now, these are placeholder values.
LONG LaserControl::GetLaserOnBitValue() const {
    // Example: assuming LASERON (Bit 2) active-LOW (0x4 if active-HIGH)
    // If set_laser_control(0x18) is used, then LASER1/2 active-LOW (Bits 3,4)
    // and LASERON is default (Bit 2).
    // For simplicity, let's assume default LASERON logic with set_laser_control(0x18)
    // This value will depend on actual laser control signals and wiring.
    // For a free-running laser, it might just be the LASERON signal.
    // Let's assume a simple scenario where we want LASERON on and other signals off/default.
    // The meaning of these bits is highly dependent on set_laser_control() and set_laser_mode().
    // A safe default for "laser on" for micro_vector_abs often corresponds to LASERON signal.
    // If using set_laser_control(0x18) which means LASER1/2 active-LOW (inverted from default),
    // and assuming LASERON is active-HIGH by default.
    // This is a placeholder that would need precise hardware configuration.
    // From RTC6 manual (page 192), set_laser_control affects LASERON, LASER1, LASER2.
    // Let's assume LASERON is Bit 0 (not 2). If LASERON is HIGH to turn on, use 1.
    // For simple demo, just assume ON = some value, OFF = 0.
    // The actual values here are bitmasks controlling LASERON, LASER1, LASER2.
    // This needs to be precisely defined based on your laser wiring and set_laser_control.
    return 0xFFFFFFFF; // All bits ON as a placeholder for "Laser On"
}

LONG LaserControl::GetLaserOffBitValue() const {
    return 0x00000000; // All bits OFF (laser off)
}

// Global Laser Configuration (called by Rtc6Controller)
void LaserControl::ConfigureGlobalLaser(UINT mode, UINT controlSignals, double markSpeed, double jumpSpeed) {
    // Update internal state
    currentLaserMode_ = mode;
    currentControlSignals_ = controlSignals;
    currentMarkSpeed_ = markSpeed;
    currentJumpSpeed_ = jumpSpeed;

    // Call RTCAPIDriver to apply these global settings
    driver_->set_laser_mode(mode);
    driver_->set_laser_control(controlSignals);
    driver_->set_mark_speed(markSpeed);
    driver_->set_jump_speed(jumpSpeed);
}