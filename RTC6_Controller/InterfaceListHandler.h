// InterfaceListHandler.h
#pragma once

#include "Geometry.h" // For the Point struct
#include <vector>
#include "RTC6impl.h" // For UINT, INT

class InterfaceListHandler {
public:
    virtual ~InterfaceListHandler() = default;

    // Workflow Management
    virtual bool setupAutoChangeMode() = 0;
    virtual void reArmAutoChange() = 0;
    virtual bool beginListPreparation() = 0;
    virtual void endListPreparation() = 0;
    virtual bool executeCurrentListAndCycle() = 0;

    // Status
    virtual bool isListBusy(UINT listIdToCheck) const = 0;
    virtual UINT getCurrentFillListId() const = 0;

    // List Command Abstractions
    virtual void addJumpAbsolute(INT x, INT y) = 0;
    virtual void addMarkAbsolute(INT x, INT y) = 0;
    virtual void addSetFocusOffset(INT offset_bits) = 0;
    virtual void addSetMarkSpeed(double speed_mm_s) = 0;
    virtual void addSetLaserPower(UINT port, UINT power) = 0;
};