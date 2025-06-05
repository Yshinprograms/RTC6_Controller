#include <iostream>

#include "DisplayUI.h"

DisplayUI::DisplayUI() {
}

void DisplayUI::printWelcomeMessage() const {
	std::cout << "---------- RTC6 Controller Program (Communication Check) ----------" << std::endl;
}

void DisplayUI::printGoodbyeMessage() const {
	std::cout << "---------- RTC6 Controller Program (Communication Check) Ended ----------" << std::endl;
}

void DisplayUI::promptForExit() const {
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.ignore();
    std::cin.get();
}
