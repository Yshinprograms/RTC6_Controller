#ifndef RTC6_COMMON_H
#define RTC6_COMMON_H

#include <stdexcept>
#include <string>
#include <cstdint>
#include <limits>
#include <vector>

// Common type definitions
typedef unsigned int UINT;
typedef int16_t RTC6_Word;
typedef uint32_t RTC6_DWord;
typedef double RTC6_Double;

// Forward declarations for commonly used classes
class IRTCAPIDriver;
class IListCommandBuilder;
class ILaserControl;
class IMicroVectorProcessor;

/**
 * @brief Laser control mode enumeration
 */
enum class LaserControlMode {
    PWM,            ///< Pulse-width modulation mode
    Analog,         ///< Analog control mode
    DirectPWM,      ///< Direct PWM mode
    DirectAnalog    ///< Direct analog mode
};

/**
 * @brief RTC6 list execution mode
 */
enum class ListExecutionMode {
    SingleShot,     ///< Execute list once
    Continuous,     ///< Execute list continuously
    SingleStep,     ///< Execute single step of the list
    SingleStepCont  ///< Execute single step continuously
};

/**
 * @brief 3D point in RTC6 coordinate system (bits)
 */
struct Point {
    int X;  ///< X coordinate in bits
    int Y;  ///< Y coordinate in bits
    int Z;  ///< Z coordinate in bits (defocus/height)

    /// Default constructor initializes all coordinates to 0
    Point(int x = 0, int y = 0, int z = 0) : X(x), Y(y), Z(z) {}
};

// Forward declarations for commonly used classes
class IRTCAPIDriver;
class IListCommandBuilder;
class ILaserControl;

/**
 * @brief Exception class for RTC6 operations
 */
class Rtc6Exception : public std::runtime_error {
public:
    /**
     * @brief Construct a new Rtc6Exception object
     * @param message Error message
     * @param errorCode Optional error code (default: 0)
     */
    explicit Rtc6Exception(const std::string& message, UINT errorCode = 0)
        : std::runtime_error(message), errorCode_(errorCode) {}

    /**
     * @brief Get the error code associated with this exception
     * @return UINT The error code
     */
    UINT getErrorCode() const { return errorCode_; }

private:
    UINT errorCode_;  ///< Numeric error code for programmatic error handling
};

/**
 * @brief Common constants and types for RTC6 operations
 */
namespace RTC6 {
    // System constants
    constexpr UINT MAX_CORRECTION_POINTS = 64;    ///< Maximum number of correction points
    constexpr UINT MAX_LASER_POWER = 1000;        ///< Maximum laser power in Watts
    constexpr UINT MIN_LASER_POWER = 0;           ///< Minimum laser power in Watts
    
    // RTC6-specific constants
    constexpr int COORDINATE_BITS = 20;           ///< Bits used for coordinate representation
    constexpr int MAX_COORDINATE = (1 << (COORDINATE_BITS - 1)) - 1;  ///< Max coordinate value
    constexpr int MIN_COORDINATE = -(1 << (COORDINATE_BITS - 1));     ///< Min coordinate value
    constexpr int MAX_Z_AXIS = 32767;             ///< Maximum Z-axis position
    constexpr int MIN_Z_AXIS = -32768;            ///< Minimum Z-axis position
    
    // Time constants (in microseconds)
    constexpr double CLOCK_CYCLE = 0.01;          ///< RTC6 clock cycle in microseconds (10ns)
    constexpr double MIN_TIME_TICK = 0.00000001;  ///< Minimum time tick (10ns)
    constexpr double MAX_TIME_TICK = 0.1;         ///< Maximum time tick (100ms)
    
    // Default values
    constexpr double DEFAULT_MARK_SPEED = 1000.0;  ///< Default marking speed in mm/s
    constexpr double DEFAULT_JUMP_SPEED = 5000.0;  ///< Default jump speed in mm/s
    constexpr double DEFAULT_LASER_POWER = 50.0;   ///< Default laser power in Watts
    constexpr double DEFAULT_PULSE_FREQ = 1000.0;  ///< Default pulse frequency in Hz
    constexpr int DEFAULT_Z_AXIS_HEIGHT = 0;       ///< Default Z-axis height
    
    // List buffer constants
    constexpr size_t DEFAULT_LIST_BUFFER_SIZE = 0x100000;  ///< Default list buffer size (1MB)
    constexpr size_t MIN_LIST_BUFFER_SIZE = 0x1000;       ///< Minimum list buffer size (4KB)
    constexpr size_t MAX_LIST_BUFFER_SIZE = 0x1000000;    ///< Maximum list buffer size (16MB)
    
    // Error codes
    enum ErrorCode {
        NO_ERROR = 0,
        NOT_INITIALIZED,
        ALREADY_INITIALIZED,
        INVALID_PARAMETER,
        LIST_BUFFER_OVERFLOW,
        HARDWARE_ERROR,
        FILE_IO_ERROR,
        TIMEOUT,
        NOT_SUPPORTED,
        UNKNOWN_ERROR = 0xFFFFFFFF
    };
    
    // Helper functions
    /**
     * @brief Convert bits to millimeters using the current correction table
     * @param bits Value in bits
     * @param dpi Dots per inch of the correction table
     * @return double Value in millimeters
     */
    inline double bitsToMillimeters(double bits, double dpi = 1000.0) {
        return (bits * 25.4) / dpi;
    }
    
    /**
     * @brief Convert millimeters to bits using the current correction table
     * @param mm Value in millimeters
     * @param dpi Dots per inch of the correction table
     * @return int Value in bits
     */
    inline int millimetersToBits(double mm, double dpi = 1000.0) {
        return static_cast<int>((mm * dpi) / 25.4 + 0.5);
    }
}

#endif // RTC6_COMMON_H