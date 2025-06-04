#ifndef I_RTC_API_DRIVER_H
#define I_RTC_API_DRIVER_H

#include "../Rtc6Common.h"
#include <cstdint>
#include <string>
#include <vector>

// Forward declarations
class IListCommandBuilder;
class ILaserControl;

/**
 * @brief Interface for low-level RTC6 driver operations with micro-vector support
 */
class IRTCAPIDriver {
public:
    virtual ~IRTCAPIDriver() = default;

    // ===== Initialization and Control =====
    virtual bool initialize() = 0;
    virtual bool isInitialized() const = 0;
    virtual void shutdown() = 0;

    // ===== Board Communication =====
    virtual bool loadCorrectionFile(const std::string& filePath, int tableNumber = 0, int dim = 2) = 0;
    virtual bool loadCorrectionData(const double* data, int tableNumber = 0, int dim = 2) = 0;
    
    // ===== List Management =====
    virtual bool startList(int listNumber = 1) = 0;
    virtual bool executeList(int listNumber = 1) = 0;
    virtual bool stopExecution() = 0;
    virtual bool releaseList(int listNumber = 1) = 0;
    virtual bool setListMode(UINT mode) = 0;
    virtual UINT getListStatus(int listNumber = 1) const = 0;
    virtual UINT getListPointer() const = 0;

    // ===== Status and Error Handling =====
    virtual UINT getLastError() const = 0;
    virtual std::string getErrorString(UINT errorCode) const = 0;
    virtual bool isBusy() const = 0;

    // ===== Configuration =====
    virtual bool setCorrectionMode(UINT correctionMode) = 0;
    virtual bool setLaserMode(UINT laserMode) = 0;
    virtual bool setLaserDelays(double laserOnDelay, double laserOffDelay) = 0;
    virtual bool setStandby(double halfPeriod, double pulseLength) = 0;
    virtual bool setJumpSpeed(double speed) = 0;
    virtual bool setMarkSpeed(double speed) = 0;
    virtual bool setLaserPower(double power) = 0;
    virtual bool setPowerCorrection(double factor) = 0;
    virtual bool setZAxisHeight(int height) = 0;

    // ===== Basic Motion Commands =====
    virtual bool jumpAbsolute(double x, double y) = 0;
    virtual bool markAbsolute(double x, double y) = 0;
    virtual bool jumpRelative(double dx, double dy) = 0;
    virtual bool markRelative(double dx, double dy) = 0;

    // ===== Micro-Vector Commands =====
    virtual void addMicroVector(int x, int y, int dt) = 0;
    virtual void addLaserSignalOnList() = 0;
    virtual void addLaserSignalOffList() = 0;
    virtual void setCurrentListLaserPower(UINT powerValue) = 0;
    virtual void setCurrentListMarkSpeed(double speedValue) = 0;
    virtual void setCurrentListZAxisHeight(int zAxisValue) = 0;

    // ===== Arc and Circle Commands =====
    virtual bool markArc(double centerX, double centerY, double radius, 
                        double startAngle, double endAngle, bool clockwise) = 0;
    virtual bool markCircle(double centerX, double centerY, double radius) = 0;

    // ===== 3D Operations =====
    virtual bool setZAxisParams(double jumpSpeed, double markSpeed) = 0;
    virtual bool setZAxisPosition(double position) = 0;
    virtual bool setZAxisRelative(double offset) = 0;

    // ===== Digital I/O =====
    virtual bool setDigitalOutput(int port, bool state) = 0;
    virtual bool getDigitalInput(int port, bool& state) const = 0;

    // ===== Timing and Synchronization =====
    virtual bool setTimeTick(double timeInSeconds) = 0;
    virtual bool waitForTime(double timeInSeconds) = 0;
    virtual bool waitForDigitalInput(int port, bool state, double timeout) = 0;

    // ===== List Buffer Management =====
    virtual bool clearList(int listNumber = 1) = 0;
    virtual bool setListBufferSize(int sizeInBytes) = 0;
    virtual int getListBufferUsage() const = 0;

    // 3D Operations
    virtual bool set3DParams(double m11, double m12, double m21, double m22) = 0;
    virtual bool setZAxisParams(double scale, double offset) = 0;

    // Digital I/O
    virtual bool setDigitalOutput(UINT outputNumber, bool state) = 0;
    virtual bool getDigitalInput(UINT inputNumber) const = 0;
    virtual bool setDigitalOutputs(UINT outputs) = 0;
    virtual UINT getDigitalInputs() const = 0;

    // Timing and Synchronization
    virtual bool setTimingParams(UINT timeBase, UINT timeMark, UINT timeLong, 
                                UINT timeShort, UINT timePoly) = 0;
    virtual bool setAutoLaserControl(bool enable) = 0;

    // List Buffer Management
    virtual UINT getFreeListMemory() const = 0;
    virtual bool setListBufferSize(UINT size) = 0;
};
