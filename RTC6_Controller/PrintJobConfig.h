// In RTC6_Controller/Header Files/PrintJobConfig.h
#pragma once

#include <string>

/**
 * @brief A simple data structure to hold all configuration parameters for a print job.
 *
 * This struct is used to pass configuration settings into the PrintController
 * in a clean and organized way.
 */
struct PrintJobConfig {
    std::string ovfFilePath;
    int recoatingDelayMs;
};