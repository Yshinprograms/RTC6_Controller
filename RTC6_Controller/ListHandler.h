#pragma once
#include "Rtc6Communicator.h"
#include "RTC6impl.h"
#include <string>
#include <vector>

class ListHandler {
public:
    ListHandler(Rtc6Communicator& communicator);

    bool setupAutoChangeMode();
    bool beginListPreparation();

    void addLaserSignalOn();
    void addLaserSignalOff();

    // Sets laser power on a specified output port (e.g., Port 0 for ANALOG OUT1).
    void addSetLaserPower(UINT port, UINT power);

    // Sets the Z-axis focus offset. The correct API call is set_defocus_list.
    void addSetZPosition(INT zPos);

    // Commands a 2D jump. Z-axis is set by a separate parameter command.
    void addJumpAbsolute(INT x, INT y);

    // Adds a micro-vector with specified on/off laser delays. Execution time is a fixed 10 us.
    void addMicroVectorAbs(INT x_target, INT y_target, INT lasOnDelay, INT lasOffDelay);

    void endListPreparation();
    bool executeCurrentListAndCycle();

    // Accurately checks if a specific list (1 or 2) is busy using read_status.
    bool isListBusy(UINT listIdToCheck) const;

    // Gets the ID of the list currently designated for filling.
    UINT getCurrentFillListId() const;

private:
    Rtc6Communicator& m_communicator;
    UINT m_currentListIdForFilling;
    UINT m_currentListIdForExecution;
    bool m_autoChangeEnabled;
    bool m_firstListExecuted;

    void switchFillListTarget();
};