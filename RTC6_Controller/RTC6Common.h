// Rtc6Common.h
#ifndef RTC6_COMMON_H
#define RTC6_COMMON_H

#include <stdexcept> // For std::runtime_error
#include <string>    // For std::string
#include <cstdint>   // For uint32_t, int32_t

// --- Common Type Aliases for RTC6 API Interaction ---
// Using fixed-width integer types for clarity and portability
typedef uint32_t UINT;  // Matches RTC6impl.h's UINT
typedef int32_t LONG;   // Matches RTC6impl.h's LONG

// --- Forward Declarations of Interfaces (for cross-cutting dependencies) ---
// These are used to avoid circular includes and to make interfaces visible.
class IRTCAPIDriver;
class IListCommandBuilder;
class ILaserControl;
class IMicroVectorProcessor;

// --- Custom Exception for RTC6 Operations ---
// Provides detailed error messages including the raw RTC6 error code.
class Rtc6Exception : public std::runtime_error {
public:
    explicit Rtc6Exception(const std::string& message, UINT errorCode = 0)
        : std::runtime_error(message), errorCode_(errorCode) {}

    UINT getErrorCode() const { return errorCode_; }

private:
    UINT errorCode_;
};

// --- Enumerations for Control Modes ---
enum class LaserControlMode {
    PWM,        ///< Pulse-width modulation mode
    Analog,     ///< Analog control mode
    QSwitch,    ///< Q-switch mode (YAG)
    FreeRunning ///< Free-running laser mode
};

enum class ListExecutionMode {
    SingleShot,     ///< Execute list once
    Continuous,     ///< Execute list continuously
    SingleStep,     ///< Execute single step of the list
    SingleStepCont  ///< Execute single step continuously (typically for debugging)
};

// --- Basic 3D Point Structure (in RTC6 "bits" coordinates) ---
// RTC6 coordinates are signed 20-bit integers, typically represented by LONG.
// Z-axis values (defocus) are typically signed 16-bit integers.
struct Point {
    LONG X; ///< X coordinate in RTC6 bits
    LONG Y; ///< Y coordinate in RTC6 bits
    LONG Z; ///< Z coordinate (defocus) in RTC6 bits

    // Constructor with default values for convenience
    Point(LONG x = 0, LONG y = 0, LONG z = 0) : X(x), Y(y), Z(z) {}
};

// --- Common Constants and Helper Functions for RTC6 Domain ---
namespace RTC6 {
    // Basic RTC6 system constants
    constexpr double CLOCK_CYCLE_MICROSECONDS = 10.0; // Fundamental RTC6 DSP clock cycle: 10 microseconds

    // Common scaling factor (example: 10000 bits per millimeter if your system is calibrated this way)
    // You will need to determine the actual calibration factor of your system (e.g., from correction file)
    // For now, this is a placeholder. A real application would read this from the board.
    constexpr double DEFAULT_BITS_PER_MM = 10000.0; // Example: 10,000 bits per mm

    // Helper functions for coordinate conversion (using default scaling for now)
    inline double BitsToMillimeters(LONG bits, double bitsPerMM = DEFAULT_BITS_PER_MM) {
        return static_cast<double>(bits) / bitsPerMM;
    }

    inline LONG MillimetersToBits(double mm, double bitsPerMM = DEFAULT_BITS_PER_MM) {
        return static_cast<LONG>(mm * bitsPerMM + 0.5); // +0.5 for rounding to nearest integer
    }

    // Default RTC6 parameter values (for control commands, for example)
    constexpr double DEFAULT_MARK_SPEED_BITS_MS = 50.0;   // In bits/ms
    constexpr double DEFAULT_JUMP_SPEED_BITS_MS = 500.0;  // In bits/ms
    constexpr UINT   DEFAULT_LASER_POWER_VALUE = 32767;   // Example 16-bit DAC range (0-65535, half power)
    constexpr int    DEFAULT_Z_AXIS_HEIGHT_BITS = 0;      // In bits
}

#endif // RTC6_COMMON_H