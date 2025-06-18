#pragma once

#include <string>

// =======================================================================
// ===          MACHINE HARDWARE CONFIGURATION                         ===
// ===   !! CRITICAL: CALIBRATE THESE VALUES FOR EACH NEW MACHINE !!   ===
// =======================================================================

namespace MachineConfig {

    // --- Primary Calibration ---
    // The number of RTC6 "bits" that correspond to 1 millimeter in the scan field.
    // This is the most important value for geometric accuracy. It must be calibrated.
    constexpr double MM_TO_BITS_CONVERSION_FACTOR = 4000.0;

    // The full path to the SCANLAB-provided field correction file (.ct5).
    // This is essential for correcting lens distortion. Leave empty if not used.
    const std::string RTC6_CORRECTION_FILE_PATH = "C:\\path\\to\\correction\\file"; // Example path


    // --- Laser Power Mapping ---
    // The maximum rated power of the connected laser source in Watts.
    // This is used to convert the OVF power value into a percentage.
    constexpr double MAX_LASER_POWER_W = 400.0;


    // --- Physical Process Simulation ---
    // The delay in milliseconds to simulate the powder recoater arm movement.
    // This value is now read from here instead of being hardcoded in PrintController.
    constexpr int RECOATING_DELAY_MS = 5000;

}