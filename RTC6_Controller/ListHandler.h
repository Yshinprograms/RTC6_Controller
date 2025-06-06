// ListHandler.h
#pragma once

#include "Rtc6Communicator.h" // To use the communicator for some operations
#include "RTC6impl.h"         // For RTC6 types (UINT, INT) and direct list commands
#include <string>             // For std::string (though not strictly used in this version's public API, good for future)
#include <vector>             // For potential future use with geometry data

class ListHandler {
public:
    // Constructor: Needs a reference to the Rtc6Communicator
    ListHandler(Rtc6Communicator& communicator);

    // Call once after RTC6 initialization to enable auto-switching between List1 and List2
    bool setupAutoChangeMode();

    // Prepares a list for adding commands.
    // listId should be 1 or 2. This becomes the m_currentListIdForFilling.
    // Returns false if the list cannot be prepared.
    bool beginListPreparation(); // Simplified: always uses the current internal fill target

    // Alternative:
    // bool beginListPreparation(UINT listId); // If you want to explicitly set which list to fill

    // Adds a "laser signal on" command to the current list.
    void addLaserSignalOn();

    // Adds a "laser signal off" command to the current list.
    void addLaserSignalOff();

    // Adds a "set laser power" command to the current list.
    void addSetLaserPower(UINT power);

    // Adds a "set Z position" command to the current list.
    void addSetZPosition(INT zPos);

    // Adds a "jump absolute" command to the current list.
    void addJumpAbsolute(INT x, INT y, INT z = 0); // z is optional, for 2D jumps

    // Adds an absolute micro-vector command to the current list.
    // dx, dy are typically *relative* displacements for micro-vectors.
    // The RTC6 'micro_vector_abs' might mean the target absolute *end point* of the micro-segment.
    // This needs to be used according to the exact RTC6 manual definition for these parameters.
    // For this simple handler, we assume the user passes parameters as RTC6 expects them.
    void addMicroVectorAbs(INT x_target, INT y_target, UINT dt); // If x_target, y_target are absolute
    // Or, if they are relative displacements:
    // void addMicroVectorRel(INT dx, INT dy, UINT dt);

    // Marks the end of the current list being prepared.
    void endListPreparation();

    // Executes the list that was most recently prepared (m_currentListIdForFilling at the time endListPreparation was called).
    // Also handles switching the fill target for the next cycle if autoChange is enabled.
    // Returns false if auto_change is not enabled or no list was properly prepared.
    bool executeCurrentListAndCycle();

    // Checks if a specific list is considered busy from the handler's perspective.
    // Relies on get_status() and the handler's knowledge of which list was last executed.
    bool isListBusy(UINT listIdToCheck) const;

    // Gets the ID of the list that should currently be used for filling.
    UINT getCurrentFillListId() const;

private:
    Rtc6Communicator& m_communicator; // Reference to the RTC6 board interface
    UINT m_currentListIdForFilling;   // Which list (1 or 2) is currently being targeted for filling
    UINT m_currentListIdForExecution; // Which list (1 or 2) was last commanded to execute
    bool m_autoChangeEnabled;
    bool m_firstListExecuted;         // To handle the initial execute_list call in auto_change mode

    // Internal helper to switch the target list for filling
    void switchFillListTarget();
};