#pragma once

#include "InterfaceCommunicator.h"
#include "InterfaceListHandler.h"
#include "InterfaceRtcApi.h"
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
class ListHandler : public InterfaceListHandler{
public:
    // Constructor: Requires a communicator to interact with the hardware.
    ListHandler(InterfaceCommunicator& communicator, InterfaceRtcApi& rtcApi);
    ~ListHandler();

    bool setupAutoChangeMode() override;
    void reArmAutoChange() override;
    bool beginListPreparation() override;
    void endListPreparation() override;
    bool executeCurrentListAndCycle() override;
    bool isListBusy(UINT listIdToCheck) const override;
    UINT getCurrentFillListId() const override;
    void addJumpAbsolute(INT x, INT y) override;
    void addMarkAbsolute(INT x, INT y) override;
    void addSetFocusOffset(INT offset_bits) override;
    void addSetMarkSpeed(double speed_mm_s) override;
    void addSetLaserPower(UINT port, UINT power) override;
    UINT getLastExecutedListId() const override;


private:
    friend class ListHandler_InteractionTest;
	friend class ListHandler_LogicTest;

    InterfaceCommunicator& m_communicator;
	InterfaceRtcApi& m_rtcApi;
    UINT m_currentListIdForFilling;         // Which list buffer (1 or 2) is the target for new commands
    UINT m_currentListIdForExecution;       // Tracks which list is currently running (or was last run)

    // Toggles the internal target list ID between 1 and 2 for ping-pong buffering.
    void switchFillListTarget();

    // Private unit conversion for internal use.
    int mmToBits(double mm) const;
    UINT m_lastExecutedListId;
};