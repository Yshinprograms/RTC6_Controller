// main.cpp
#include <iostream>
#include <string>
#include <Windows.h>  // For Sleep, if still needed for user input delay

#include "Rtc6Communicator.h"
#include "DisplayUI.h"

int main() {
    // Use board ID 1 for single board setup
	Rtc6Communicator rtcCommunicator(1); 
    DisplayUI ui;

	ui.printWelcomeMessage();

	rtcCommunicator.initializeAndShowBoardInfo();

	ui.printGoodbyeMessage();
    ui.promptForExit();

    // Destructor ~Rtc6Communicator() is automatically called here when 'rtcCommunicator'
    // goes out of scope, ensuring free_rtc6_dll() is called if it was initialized.
    return 0;
}