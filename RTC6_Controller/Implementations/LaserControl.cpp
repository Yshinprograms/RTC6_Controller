
#include "LaserControl.h"
#include <stdexcept>

LaserControl::LaserControl(IRTCAPIDriver* driver)
    : m_driver(driver) {
    if (m_driver == nullptr) {
        throw std::invalid_argument("IRTCAPIDriver pointer cannot be null");
    }
}

// Basic laser control implementation
void LaserControl::SetPower(UINT power) {
    m_currentPower = power;
    // In a real implementation, this would call the appropriate driver method
    // m_driver->SetLaserPower(power);
}

UINT LaserControl::GetPower() const {
    return m_currentPower;
}

void LaserControl::SetPulseFrequency(double frequency) {
    m_currentFrequency = frequency;
    // In a real implementation, this would call the appropriate driver method
    // m_driver->SetPulseFrequency(frequency);
}

double LaserControl::GetPulseFrequency() const {
    return m_currentFrequency;
}

void LaserControl::EnableLaser() {
    m_laserEnabled = true;
    // In a real implementation, this would call the appropriate driver method
    // m_driver->EnableLaser();
}

void LaserControl::DisableLaser() {
    m_laserEnabled = false;
    // In a real implementation, this would call the appropriate driver method
    // m_driver->DisableLaser();
}

bool LaserControl::IsLaserEnabled() const {
    return m_laserEnabled;
}

// List-based parameter control implementation
UINT LaserControl::GetPowerForList() const {
    // Return a default value for demonstration
    // In a real implementation, this would return the current power setting for list mode
    return 10000; // 100% power as an example value
}

double LaserControl::GetSpeedForList() const {
    // Return a default value for demonstration
    // In a real implementation, this would return the current speed setting for list mode
    return 50.0; // 50 mm/s as an example value
}

int LaserControl::GetZAxisHeightForList() const {
    // Return a default value for demonstration
    // In a real implementation, this would return the current Z-axis height for list mode
    return 0; // 0 mm as an example value
}

// Advanced control implementation
void LaserControl::SetLaserControlMode(LaserControlMode mode) {
    m_controlMode = mode;
    // In a real implementation, this would configure the driver for the specified mode
    // m_driver->SetLaserControlMode(mode);
}

LaserControlMode LaserControl::GetLaserControlMode() const {
    return m_controlMode;
}

void LaserControl::SetAnalogInputRange(double minVoltage, double maxVoltage) {
    if (minVoltage >= maxVoltage) {
        throw std::invalid_argument("Minimum voltage must be less than maximum voltage");
    }
    m_analogMinVoltage = minVoltage;
    m_analogMaxVoltage = maxVoltage;
    // In a real implementation, this would configure the analog input range on the hardware
    // m_driver->SetAnalogInputRange(minVoltage, maxVoltage);
}

void LaserControl::GetAnalogInputRange(double& minVoltage, double& maxVoltage) const {
    minVoltage = m_analogMinVoltage;
    maxVoltage = m_analogMaxVoltage;
}
