// Interfaces/IListCommandBuilder.h
#ifndef I_LIST_COMMAND_BUILDER_H
#define I_LIST_COMMAND_BUILDER_H

#include "Rtc6Common.h" // For Point, UINT, LONG types

// Interface for building RTC6 command lists.
// This interface defines methods for adding various types of commands to the
// RTC6 board's internal list memory, to be executed by the DSP.
class IListCommandBuilder {
public:
    virtual ~IListCommandBuilder() = default;

    // --- List Management ---
    // These methods control the active list and its execution state.
    virtual void SetStartList(UINT listNumber) = 0; // Prepares a list for writing
    virtual void SetEndOfList() = 0;                // Marks the end of commands in a list
    virtual void ExecuteList(UINT listNumber) = 0;  // Starts execution of a list

    // --- Basic Motion Commands (for building polygons/paths) ---
    // These commands are added to the list and executed by the DSP.
    virtual void JumpAbsolute(LONG x, LONG y) = 0; // Move laser off
    virtual void MarkAbsolute(LONG x, LONG y) = 0; // Move laser on

    // --- Micro-Vector Specific Command ---
    // Adds a single micro-vector segment to the list.
    // Laser state is controlled directly by laserOnBits/laserOffBits for this 10us segment.
    virtual void AddMicroVector(LONG x, LONG y, LONG laserOnBits, LONG laserOffBits) = 0;

    // --- Dynamic Parameter Control (List-based) ---
    // These commands are added to the list and change laser/scanner parameters
    // for subsequent commands within the same list.
    virtual void SetCurrentListLaserPower(UINT powerValue) = 0;
    virtual void SetCurrentListMarkSpeed(double speedValue) = 0;
    virtual void SetCurrentListZAxisHeight(LONG zAxisValue) = 0;
};

#endif // I_LIST_COMMAND_BUILDER_H