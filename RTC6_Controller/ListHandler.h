#pragma once

#include "Rtc6Communicator.h"
#include "RTC6impl.h"
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// ListHandler Class
// -----------------------------------------------------------------------------
// Purpose:
// Encapsulates all logic related to creating, managing, and executing RTC6
// command lists. It provides a simplified interface for common list operations
// and handles the complexities of the ping-pong buffering (auto-change) workflow.
// -----------------------------------------------------------------------------
class ListHandler {
public:
    // Constructor: Requires a communicator to interact with the hardware.
    ListHandler(Rtc6Communicator& communicator);

    // --- Workflow Management ---

    // Arms the auto-change mechanism. Must be called once before the first
    // list is executed and then re-armed for each subsequent list.
    bool setupAutoChangeMode();
    void reArmAutoChange();

    // Marks the beginning of a command list for filling.
    bool beginListPreparation();

    // Marks the end of a command list.
    void endListPreparation();

    // Commands the execution of the list that was just prepared and cycles
    // the internal state to prepare for filling the next list.
    bool executeCurrentListAndCycle();

    // --- Status & State ---

    // Checks if a specific list (1 or 2) is currently being processed by the hardware.
    // Uses read_status for accurate, real-time feedback.
    bool isListBusy(UINT listIdToCheck) const;

    // Returns the ID of the list currently designated for filling.
    UINT getCurrentFillListId() const;


    // --- List Command Abstractions ---

    // Adds a 2D jump command (laser off).
    void addJumpAbsolute(INT x, INT y);

    // Adds a 2D mark command (laser on, respects mark speed).
    void addMarkAbsolute(INT x, INT y);

    // Sets the Z-axis optical focus offset.
    void addSetFocusOffset(INT offset_bits);

    // Sets the marking speed for subsequent mark/arc commands.
    void addSetMarkSpeed(double speed_mm_s);

    // Sets laser power on a specified output port (e.g., Port 1 for ANALOG OUT1).
    void addSetLaserPower(UINT port, UINT power);


private:
    Rtc6Communicator& m_communicator;       // Reference to the hardware interface
    UINT m_currentListIdForFilling;         // Which list buffer (1 or 2) is the target for new commands
    UINT m_currentListIdForExecution;       // Tracks which list is currently running (or was last run)

    // Toggles the internal target list ID between 1 and 2 for ping-pong buffering.
    void switchFillListTarget();

    // Private unit conversion for internal use.
    int mmToBits(double mm) const;
};