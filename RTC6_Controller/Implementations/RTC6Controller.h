#pragma once

#include "../Interfaces/IRTCAPIDriver.h"
#include "../Interfaces/IListCommandBuilder.h"
#include "../Interfaces/ILaserControl.h"
#include <memory>
#include <string>

/**
 * @brief High-level controller for RTC6 laser marking system
 * 
 * This class provides a high-level interface for controlling the RTC6 laser system,
 * coordinating between the low-level driver, command building, and laser control components.
 */
class RTC6Controller {
public:
    /**
     * @brief Construct a new RTC6Controller
     * 
     * @param driver The RTC6 driver instance to use
     * @param commandBuilder The command builder to use
     * @param laserControl The laser control instance to use
     */
    RTC6Controller(
        std::unique_ptr<IRTCAPIDriver> driver,
        std::unique_ptr<IListCommandBuilder> commandBuilder,
        std::unique_ptr<ILaserControl> laserControl
    );
    
    ~RTC6Controller();
    
    // Prevent copying
    RTC6Controller(const RTC6Controller&) = delete;
    RTC6Controller& operator=(const RTC6Controller&) = delete;
    
    // System Control
    bool initialize();
    bool isInitialized() const;
    void shutdown();
    
    // Configuration
    bool loadCorrectionFile(const std::string& filePath);
    bool setLaserPower(double power);
    bool setMarkingParams(double speed, double power);
    
    // Marking Operations
    bool markPoint(double x, double y);
    bool markLine(double x1, double y1, double x2, double y2);
    bool markRectangle(double x, double y, double width, double height);
    bool markCircle(double centerX, double centerY, double radius);
    
    // Batch Operations
    bool startMarking();
    bool executeMarking();
    bool stopMarking();
    
    // Status
    std::string getLastError() const;
    bool isBusy() const;
    
    // Access to underlying components
    IRTCAPIDriver& getDriver();
    IListCommandBuilder& getCommandBuilder();
    ILaserControl& getLaserControl();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};
