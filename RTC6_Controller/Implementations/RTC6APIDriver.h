#pragma once

#include "../Interfaces/IRTCAPIDriver.h"
#include <string>
#include <memory>

// Forward declaration of implementation details
struct RTC6APIDriverImpl;

/**
 * @brief Concrete implementation of the IRTCAPIDriver interface for RTC6 controller
 */
class RTC6APIDriver : public IRTCAPIDriver {
public:
    RTC6APIDriver();
    ~RTC6APIDriver() override;

    // Prevent copying and assignment
    RTC6APIDriver(const RTC6APIDriver&) = delete;
    RTC6APIDriver& operator=(const RTC6APIDriver&) = delete;

    // IRTCAPIDriver implementation
    bool initialize() override;
    bool isInitialized() const override;
    void shutdown() override;

    // Board Communication
    bool loadCorrectionFile(const std::string& filePath, int tableNumber, int dim) override;
    bool loadCorrectionData(const double* data, int tableNumber, int dim) override;
    
    // List Management
    bool startList() override;
    bool executeList() override;
    bool stopExecution() override;
    bool releaseList() override;

    // Status and Error Handling
    UINT getLastError() const override;
    std::string getErrorString(UINT errorCode) const override;
    bool isBusy() const override;

    // Configuration
    bool setCorrectionMode(UINT correctionMode) override;
    bool setLaserMode(UINT laserMode) override;
    bool setLaserDelays(double laserOnDelay, double laserOffDelay) override;
    bool setStandby(double halfPeriod, double pulseLength) override;

    // Marking Operations
    bool markPoint(double x, double y) override;
    bool markLine(double startX, double startY, double endX, double endY) override;
    bool markArc(double centerX, double centerY, double radius, double startAngle, 
                double endAngle, bool clockwise) override;

    // Advanced Features
    bool setJumpSpeed(double speed) override;
    bool setMarkSpeed(double speed) override;
    bool setLaserPower(double power) override;
    bool setPowerCorrection(double factor) override;

    // 3D Operations
    bool set3DParams(double m11, double m12, double m21, double m22) override;
    bool setZAxisParams(double scale, double offset) override;

    // Digital I/O
    bool setDigitalOutput(UINT outputNumber, bool state) override;
    bool getDigitalInput(UINT inputNumber) const override;
    bool setDigitalOutputs(UINT outputs) override;
    UINT getDigitalInputs() const override;

    // Timing and Synchronization
    bool setTimingParams(UINT timeBase, UINT timeMark, UINT timeLong, 
                        UINT timeShort, UINT timePoly) override;
    bool setAutoLaserControl(bool enable) override;

    // List Buffer Management
    UINT getFreeListMemory() const override;
    bool setListBufferSize(UINT size) override;

private:
    std::unique_ptr<RTC6APIDriverImpl> pImpl;  // PIMPL idiom for hiding implementation details
};
