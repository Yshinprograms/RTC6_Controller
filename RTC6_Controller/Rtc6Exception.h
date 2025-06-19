#pragma once
#include <stdexcept>
#include <string>

/**
 * @brief Base class for all custom exceptions thrown by the RTC6_Controller library.
 *
 * Inheriting from std::runtime_error allows these exceptions to be handled
 * by standard C++ exception mechanisms and provides the .what() method.
 */
class Rtc6Exception : public std::runtime_error {
public:
    explicit Rtc6Exception(const std::string& message)
        : std::runtime_error(message) {}
};


/**
 * @brief Thrown when an error occurs during communication with the RTC6 hardware.
 */
class HardwareError : public Rtc6Exception {
public:
    explicit HardwareError(const std::string& message)
        : Rtc6Exception("Hardware Error: " + message) {
    }
};


/**
 * @brief Thrown when an error occurs during the parsing of an OVF file.
 */
class FileParseError : public Rtc6Exception {
public:
    explicit FileParseError(const std::string& message)
        : Rtc6Exception("File Parse Error: " + message) {
    }
};


/**
 * @brief Thrown when a logical or configuration error is detected.
 *
 * e.g., A vector block references a parameter key that does not exist in the job shell.
 */
class ConfigurationError : public Rtc6Exception {
public:
    explicit ConfigurationError(const std::string& message)
        : Rtc6Exception("Configuration Error: " + message) {
    }
};