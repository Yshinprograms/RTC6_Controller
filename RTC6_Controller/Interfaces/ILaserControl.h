#ifndef I_LASER_CONTROL_H
#define I_LASER_CONTROL_H

#include "Rtc6Common.h"

/**
 * @brief Interface for laser parameter control with micro-vector support
 */
class ILaserControl {
public:
    virtual ~ILaserControl() = default;

    // Basic laser control
    virtual void SetPower(UINT power) = 0;
    virtual UINT GetPower() const = 0;
    virtual void SetPulseFrequency(double frequency) = 0;
    virtual double GetPulseFrequency() const = 0;
    virtual void EnableLaser() = 0;
    virtual void DisableLaser() = 0;
    virtual bool IsLaserEnabled() const = 0;

    // List-based parameter control
    virtual UINT GetPowerForList() const = 0;
    virtual double GetSpeedForList() const = 0;
    virtual int GetZAxisHeightForList() const = 0;

    // Advanced control
    virtual void SetLaserControlMode(LaserControlMode mode) = 0;
    virtual LaserControlMode GetLaserControlMode() const = 0;
    virtual void SetAnalogInputRange(double minVoltage, double maxVoltage) = 0;
    virtual void GetAnalogInputRange(double& minVoltage, double& maxVoltage) const = 0;
};

#endif // I_LASER_CONTROL_H
