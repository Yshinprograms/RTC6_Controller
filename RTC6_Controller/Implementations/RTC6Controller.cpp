#include "RTC6Controller.h"
#include "../RTC6Common.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>

// PIMPL implementation
class RTC6Controller::Impl {
public:
    Impl(
        std::unique_ptr<IRTCAPIDriver> driver,
        std::unique_ptr<IListCommandBuilder> commandBuilder,
        std::unique_ptr<ILaserControl> laserControl
    ) : driver_(std::move(driver)),
        commandBuilder_(std::move(commandBuilder)),
        laserControl_(std::move(laserControl)),
        isInitialized_(false) {}

    ~Impl() {
        if (isInitialized_) {
            shutdown();
        }
    }

    bool initialize() {
        if (isInitialized_) return true;

        try {
            if (!driver_->initialize()) {
                lastError_ = "Failed to initialize RTC6 driver: " + 
                            driver_->getErrorString(driver_->getLastError());
                return false;
            }

            // Load default correction file if available
            // This can be overridden by the user later
            if (!loadCorrectionFile("C:\\Program Files\\SCAPS\\Cor_1to1.ct5")) {
                // Non-fatal error, just log it
                lastError_ = "Warning: Could not load default correction file";
            }

            isInitialized_ = true;
            return true;
        } catch (const std::exception& e) {
            lastError_ = std::string("Initialization error: ") + e.what();
            return false;
        }
    }

    bool isInitialized() const {
        return isInitialized_ && driver_->isInitialized();
    }

    void shutdown() {
        if (isInitialized_) {
            try {
                stopMarking();
                driver_->shutdown();
            } catch (...) {
                // Ensure we don't throw from destructor
            }
            isInitialized_ = false;
        }
    }

    bool loadCorrectionFile(const std::string& filePath) {
        if (!isInitialized_) return false;
        
        // Try both 2D and 3D correction tables
        if (driver_->loadCorrectionFile(filePath, 1, 2) ||  // 2D
            driver_->loadCorrectionFile(filePath, 1, 3)) {   // 3D
            return true;
        }
        
        lastError_ = "Failed to load correction file: " + filePath;
        return false;
    }

    bool setLaserPower(double power) {
        if (!isInitialized_) return false;
        
        // Delegate to laser control if available
        if (laserControl_) {
            return laserControl_->setLaserPower(power);
        }
        
        // Fallback to direct driver control
        return driver_->setLaserPower(power);
    }

    bool setMarkingParams(double speed, double power) {
        if (!isInitialized_) return false;
        
        bool success = true;
        
        // Set jump and mark speeds (assuming same speed for both in this example)
        success &= driver_->setJumpSpeed(speed);
        success &= driver_->setMarkSpeed(speed);
        
        // Set laser power
        success &= setLaserPower(power);
        
        if (!success) {
            lastError_ = "Failed to set marking parameters";
        }
        
        return success;
    }

    // Marking operations
    bool markPoint(double x, double y) {
        if (!isInitialized_) return false;
        
        if (!driver_->startList()) {
            lastError_ = "Failed to start command list";
            return false;
        }
        
        bool success = driver_->markPoint(x, y);
        
        if (success) {
            success = driver_->executeList();
        }
        
        if (!success) {
            lastError_ = "Marking operation failed";
        }
        
        return success;
    }
    
    bool markLine(double x1, double y1, double x2, double y2) {
        if (!isInitialized_) return false;
        
        if (!driver_->startList()) {
            lastError_ = "Failed to start command list";
            return false;
        }
        
        bool success = driver_->markLine(x1, y1, x2, y2);
        
        if (success) {
            success = driver_->executeList();
        }
        
        if (!success) {
            lastError_ = "Line marking failed";
        }
        
        return success;
    }
    
    bool markRectangle(double x, double y, double width, double height) {
        if (!isInitialized_) return false;
        
        if (!driver_->startList()) {
            lastError_ = "Failed to start command list";
            return false;
        }
        
        // Draw four lines to form a rectangle
        bool success = true;
        success &= driver_->markLine(x, y, x + width, y);                  // Top
        success &= driver_->markLine(x + width, y, x + width, y + height);  // Right
        success &= driver_->markLine(x + width, y + height, x, y + height); // Bottom
        success &= driver_->markLine(x, y + height, x, y);                   // Left
        
        if (success) {
            success = driver_->executeList();
        }
        
        if (!success) {
            lastError_ = "Rectangle marking failed";
        }
        
        return success;
    }
    
    bool markCircle(double centerX, double centerY, double radius) {
        if (!isInitialized_) return false;
        
        // For simplicity, we'll use an octagonal approximation
        // A more accurate implementation would use multiple small line segments
        const int segments = 32;
        const double angleIncrement = 2.0 * 3.14159265358979323846 / segments;
        
        if (!driver_->startList()) {
            lastError_ = "Failed to start command list";
            return false;
        }
        
        // Move to starting point
        double startX = centerX + radius;
        double startY = centerY;
        
        bool success = true;
        for (int i = 1; i <= segments; ++i) {
            double angle = i * angleIncrement;
            double endX = centerX + radius * cos(angle);
            double endY = centerY + radius * sin(angle);
            
            success &= driver_->markLine(startX, startY, endX, endY);
            
            startX = endX;
            startY = endY;
        }
        
        if (success) {
            success = driver_->executeList();
        }
        
        if (!success) {
            lastError_ = "Circle marking failed";
        }
        
        return success;
    }
    
    // Batch operations
    bool startMarking() {
        if (!isInitialized_) return false;
        return driver_->startList();
    }
    
    bool executeMarking() {
        if (!isInitialized_) return false;
        return driver_->executeList();
    }
    
    bool stopMarking() {
        if (!isInitialized_) return false;
        
        bool success = true;
        success &= driver_->stopExecution();
        success &= driver_->releaseList();
        
        return success;
    }
    
    // Status
    std::string getLastError() const {
        return lastError_;
    }
    
    bool isBusy() const {
        return isInitialized_ && driver_->isBusy();
    }
    
    // Component accessors
    IRTCAPIDriver& getDriver() {
        return *driver_;
    }
    
    IListCommandBuilder& getCommandBuilder() {
        return *commandBuilder_;
    }
    
    ILaserControl& getLaserControl() {
        return *laserControl_;
    }

private:
    std::unique_ptr<IRTCAPIDriver> driver_;
    std::unique_ptr<IListCommandBuilder> commandBuilder_;
    std::unique_ptr<ILaserControl> laserControl_;
    bool isInitialized_;
    mutable std::string lastError_;
};

// RTC6Controller implementation
RTC6Controller::RTC6Controller(
    std::unique_ptr<IRTCAPIDriver> driver,
    std::unique_ptr<IListCommandBuilder> commandBuilder,
    std::unique_ptr<ILaserControl> laserControl
) : pImpl(std::make_unique<Impl>(
    std::move(driver), 
    std::move(commandBuilder), 
    std::move(laserControl)
)) {}

RTC6Controller::~RTC6Controller() = default;

bool RTC6Controller::initialize() { return pImpl->initialize(); }
bool RTC6Controller::isInitialized() const { return pImpl->isInitialized(); }
void RTC6Controller::shutdown() { pImpl->shutdown(); }
bool RTC6Controller::loadCorrectionFile(const std::string& filePath) { return pImpl->loadCorrectionFile(filePath); }
bool RTC6Controller::setLaserPower(double power) { return pImpl->setLaserPower(power); }
bool RTC6Controller::setMarkingParams(double speed, double power) { return pImpl->setMarkingParams(speed, power); }
bool RTC6Controller::markPoint(double x, double y) { return pImpl->markPoint(x, y); }
bool RTC6Controller::markLine(double x1, double y1, double x2, double y2) { return pImpl->markLine(x1, y1, x2, y2); }
bool RTC6Controller::markRectangle(double x, double y, double width, double height) { return pImpl->markRectangle(x, y, width, height); }
bool RTC6Controller::markCircle(double centerX, double centerY, double radius) { return pImpl->markCircle(centerX, centerY, radius); }
bool RTC6Controller::startMarking() { return pImpl->startMarking(); }
bool RTC6Controller::executeMarking() { return pImpl->executeMarking(); }
bool RTC6Controller::stopMarking() { return pImpl->stopMarking(); }
std::string RTC6Controller::getLastError() const { return pImpl->getLastError(); }
bool RTC6Controller::isBusy() const { return pImpl->isBusy(); }
IRTCAPIDriver& RTC6Controller::getDriver() { return pImpl->getDriver(); }
IListCommandBuilder& RTC6Controller::getCommandBuilder() { return pImpl->getCommandBuilder(); }
ILaserControl& RTC6Controller::getLaserControl() { return pImpl->getLaserControl(); }
	UINT selectedCard = 0;
	std::cout << "\n[Step 3] Attempting to select RTC6 board " << BOARD_ID << "..." << std::endl;
	selectedCard = select_rtc(selectedCard); // `select_rtc` returns the card number if successful, 0 if not.
	if (selectedCard == 0) {
		std::cerr << "FATAL ERROR: Failed to select RTC6 board. Check card number or access rights." << std::endl;
		CheckRtcError(selectedCard, "select_rtc"); 
		free_rtc6_dll();
		return 1;
	}
	selectedCard = BOARD_ID;
	std::cout << "RTC6 board " << selectedCard << " selected successfully." << std::endl;

	// Step 4: Load the RTC6 DSP firmware (Ensure out, rbf, and dat file are in executable directory)
	std::cout << "\n[Step 4] Loading RTC6 DSP firmware..." << std::endl;
	errorCode = load_program_file(0);
	if (errorCode != 0) {
		std::cerr << "FATAL ERROR: Failed to load DSP program file. Error Code: " << errorCode << std::endl;
		CheckRtcError(selectedCard, "load_program_file");
		std::cerr << "       Ensure RTC6OUT.out, RTC6RBF.rbf, RTC6DAT.dat are in the executable directory and are compatible with DLL." << std::endl;
		free_rtc6_dll();
		return 1;
	}
	std::cout << "DSP program file loaded successfully." << std::endl;

	// Step 5: Get RTC firmware version
	UINT rtcVersion = get_rtc_version();
	CheckRtcError(selectedCard, "get_rtc_version"); // Always check errors!
	std::cout << "\n[Step 5] Board " << selectedCard << " RTC Firmware Version: " << rtcVersion << std::endl;

	// Step 6: Get BIOS version
	UINT biosVersion = get_bios_version();
	CheckRtcError(selectedCard, "get_bios_version");
	std::cout << "\n[Step 6] Board " << selectedCard << " BIOS Version: " << biosVersion << std::endl;

	// Step 7: Get board serial number
	UINT serialNumber = get_serial_number();
	CheckRtcError(selectedCard, "n_get_serial_number");
	std::cout << "\n[Step 7] Board " << selectedCard << " Serial Number: " << serialNumber << std::endl;

	// Check for any accumulated errors that might not have been caught by individual `CheckRtcError` calls.
	UINT32 accumulatedError = get_error();
	if (accumulatedError != 0) {
		std::cerr << "WARNING: Accumulated errors on board " << selectedCard << ". Error Code: " << std::bitset<32>(accumulatedError) << std::endl;
		std::cout << "       Bit 3 will be set if there is no program uploaded onto RTC6" << std::endl;
	}

	std::cout << "\nRTC6 Handshake successful! Board is ready for further commands." << std::endl;

	// Keep console open to see output
	std::cout << "\nPress Enter to exit..." << std::endl;
	std::cin.ignore();
	std::cin.get();

	// Step 8: Release RTC6 DLL resources
	std::cout << "Releasing RTC6 DLL resources..." << std::endl;
	free_rtc6_dll();
	std::cout << "Resources released. Program exiting." << std::endl;

	return 0;
}
