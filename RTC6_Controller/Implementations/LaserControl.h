#pragma once
#include "../Interfaces/ILaserControl.h"

// Forward declaration
class IRTCAPIDriver;

class LaserControl : public ILaserControl {
public:
    explicit LaserControl(IRTCAPIDriver* driver);
    ~LaserControl() override = default;

    // Basic laser control
    void SetPower(UINT power) override;
    UINT GetPower() const override;
    void SetPulseFrequency(double frequency) override;
    double GetPulseFrequency() const override;
    void EnableLaser() override;
    void DisableLaser() override;
    bool IsLaserEnabled() const override;

    // List-based parameter control
    UINT GetPowerForList() const override;
    double GetSpeedForList() const override;
    int GetZAxisHeightForList() const override;

    // Advanced control
    void SetLaserControlMode(LaserControlMode mode) override;
    LaserControlMode GetLaserControlMode() const override;
    void SetAnalogInputRange(double minVoltage, double maxVoltage) override;
    void GetAnalogInputRange(double& minVoltage, double& maxVoltage) const override;

private:
    IRTCAPIDriver* m_driver;
    UINT m_currentPower{0};
    double m_currentFrequency{0.0};
    bool m_laserEnabled{false};
    LaserControlMode m_controlMode{LaserControlMode::Power};
    double m_analogMinVoltage{0.0};
    double m_analogMaxVoltage{5.0};
};
