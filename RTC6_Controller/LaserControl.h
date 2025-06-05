// Implementations/LaserControl.h
#ifndef LASER_CONTROL_H
#define LASER_CONTROL_H

#include "ILaserControl.h" // Inherit from the interface
#include "IRTCAPIDriver.h" // For injected driver dependency
#include <stdexcept>                     // For std::invalid_argument

// Concrete implementation of ILaserControl.
// Manages laser parameters and translates them into RTC6 API calls.
class LaserControl : public ILaserControl {
public:
    // Constructor: Takes a non-owning pointer to the low-level RTC6 driver.
    explicit LaserControl(IRTCAPIDriver* driver);

    // Destructor (defaulted).
    ~LaserControl() override = default;

    // Prevent copying and assignment
    LaserControl(const LaserControl&) = delete;
    LaserControl& operator=(const LaserControl&) = delete;

    // --- Implementations of ILaserControl methods ---

    // Getters for List-based Parameter Values
    UINT GetPowerForList() const override;
    double GetSpeedForList() const override;
    LONG GetZAxisHeightForList() const override;

    // Getters for Laser Control Bitmasks (for micro_vector_abs/rel)
    LONG GetLaserOnBitValue() const override;
    LONG GetLaserOffBitValue() const override;

    // Global Laser Configuration (called by Rtc6Controller)
    void ConfigureGlobalLaser(UINT mode, UINT controlSignals, double markSpeed, double jumpSpeed) override;

private:
    IRTCAPIDriver* driver_; // Non-owning pointer to the low-level RTC6 API driver.

    // Internal state to store the currently configured parameters
    // These would typically be updated via UI or configuration files in a full application.
    UINT currentLaserMode_ = 0;        // Default CO2 mode
    UINT currentControlSignals_ = 0x18; // Default active-LOW LASER1/LASER2
    double currentMarkSpeed_ = RTC6::DEFAULT_MARK_SPEED_BITS_MS;
    double currentJumpSpeed_ = RTC6::DEFAULT_JUMP_SPEED_BITS_MS;
    UINT currentLaserPowerValue_ = RTC6::DEFAULT_LASER_POWER_VALUE; // Default for DAC
    LONG currentZAxisHeight_ = RTC6::DEFAULT_Z_AXIS_HEIGHT_BITS;
};

#endif // LASER_CONTROL_H