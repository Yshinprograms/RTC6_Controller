// DisplayUI.h
#pragma once

#include <string> // For std::string to allow customizable messages/titles

class DisplayUI {
public:
    // Constructor (optional, could be empty if no setup needed)
    DisplayUI();

    // Prints a standard welcome banner for the program.
    // Takes the program title as an argument for customization.
    void printWelcomeMessage() const;

    // Prints a standard goodbye message.
    void printGoodbyeMessage() const;

    // Handles the "Press Enter to exit..." prompt and waits for user input.
    void promptForExit() const;

    // You could add more methods here later for other UI elements, like:
    // void printError(const std::string& errorMessage) const;
    // void printStatus(const std::string& statusMessage) const;
};