#pragma once

#include "InterfaceUI.h"
#include <string>

/**
 * @brief A concrete implementation of the IUserInterface that prints to the standard console.
 *
 * This class is specific to the RTC6_Main application and handles all user
 * interaction via std::cout and std::cerr.
 */
class ConsoleUI : public InterfaceUI {
public:
    // --- IUserInterface Contract Implementation ---
    void displayMessage(const std::string& message) override;
    void displayError(const std::string& message) override;
    void displayProgress(const std::string& message, int current, int total) override;

    // --- Application-Specific Methods ---
    // These methods are not part of the interface because they are specific
    // to a console application's lifecycle.
    void printWelcomeMessage();
    void printGoodbyeMessage();
    void promptForExit();
};