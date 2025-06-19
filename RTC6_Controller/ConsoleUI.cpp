#include "ConsoleUI.h"
#include <iostream>

void ConsoleUI::displayMessage(const std::string& message) {
    std::cout << message << std::endl;
}

void ConsoleUI::displayError(const std::string& message) {
    // It's good practice to send errors to the standard error stream.
    std::cerr << "ERROR: " << message << std::endl;
}

void ConsoleUI::displayProgress(const std::string& message, int current, int total) {
    // Adding 1 to 'current' makes the output more human-readable (e.g., "1 / 10" instead of "0 / 10").
    std::cout << "[ " << current + 1 << " / " << total << " ] " << message << std::endl;
}

void ConsoleUI::printWelcomeMessage() {
    std::cout << "========================================" << std::endl;
    std::cout << "==   RTC6 OVF Print Controller        ==" << std::endl;
    std::cout << "========================================" << std::endl;
}

void ConsoleUI::printGoodbyeMessage() {
    std::cout << "========================================" << std::endl;
    std::cout << "==   Print Job Finished               ==" << std::endl;
    std::cout << "========================================" << std::endl;
}

void ConsoleUI::promptForExit() {
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.get();
}