// Interfaces/ILaserControl.h
#ifndef I_LASER_CONTROL_H
#define I_LASER_CONTROL_H

#include "Rtc6Common.h" // For UINT, LaserControlMode

// Interface for laser parameter control.
// This interface defines how higher-level components (like geometry processors)
// can query the current laser parameters to embed them into command lists,
// and how overall laser configuration can be managed.
class ILaserControl {
public:
    virtual ~ILaserControl() = default;

    // --- Getters for List-based Parameter Values ---
    // These methods provide the values that should be embedded in RTC6 list commands.
    // They reflect the *current configured* laser/scanner settings.
    virtual UINT GetPowerForList() const = 0;       // Returns the laser power value (e.g., DAC value)
    virtual double GetSpeedForList() const = 0;     // Returns the desired mark speed in bits/ms
    virtual LONG GetZAxisHeightForList() const = 0; // Returns the desired Z-axis height in bits

    // --- Getters for Laser Control Bitmasks (for micro_vector_abs/rel) ---
    // These provide the bitmasks for LASER1, LASER2, LASERON signals when laser is ON/OFF.
    // Actual values depend on RTC6's set_laser_control and laser mode.
    virtual LONG GetLaserOnBitValue() const = 0;  // Bitmask for laser ON state
    virtual LONG GetLaserOffBitValue() const = 0; // Bitmask for laser OFF state

    // --- Global Laser Configuration (set by Rtc6Controller) ---
    // These methods allow Rtc6Controller to configure the laser.
    // They are not typically used by ListCommandBuilder or MicroVectorProcessor.
    virtual void ConfigureGlobalLaser(UINT mode, UINT controlSignals, double markSpeed, double jumpSpeed) = 0;
};

#endif // I_LASER_CONTROL_H