// Implementations/RTC6Controller.h
#ifndef RTC6_CONTROLLER_H
#define RTC6_CONTROLLER_H

#include "IRTCAPIDriver.h"        // Injected dependency (for board comms)
#include "IListCommandBuilder.h"   // Injected dependency (for list building)
#include "ILaserControl.h"         // Injected dependency (for laser params)
#include "Rtc6Common.h"                       // For custom types, exceptions
#include <memory>                                // For std::unique_ptr
#include <string>                                // For std::string

// High-level controller for a single RTC6 board.
// Orchestrates multiple low-level driver calls to perform complex operations,
// and manages the board's overall state and configuration.
class RTC6Controller {
public:
    // Constructor: Takes unique_ptrs to the interfaces, indicating ownership transfer.
    // This adheres to Dependency Injection and ensures proper resource management.
    RTC6Controller(std::unique_ptr<IRTCAPIDriver> driver,
        std::unique_ptr<IListCommandBuilder> commandBuilder,
        std::unique_ptr<ILaserControl> laserControl);

    // Destructor (defaulted, as unique_ptrs handle resource cleanup).
    ~RTC6Controller();

    // Prevent copying and assignment (important for classes owning unique resources)
    RTC6Controller(const RTC6Controller&) = delete;
    RTC6Controller& operator=(const RTC6Controller&) = delete;
    RTC6Controller(RTC6Controller&&) = delete;
    RTC6Controller& operator=(RTC6Controller&&) = delete;

    // --- High-Level System Control & Initialization ---
    // Orchestrates DLL initialization, board selection, and firmware loading.
    void InitializeAndLoadFirmware(UINT boardId = 1);

    // Queries and retrieves various board identification and version information.
    void QueryBoardInformation(UINT boardId, UINT& rtcVersion, UINT& biosVersion, UINT& serialNumber);

    // Configures global laser parameters (mode, control signals, default speeds).
    void ConfigureGlobalLaser(UINT mode, UINT controlSignals, double markSpeed, double jumpSpeed);

    // --- High-Level List Management ---
    // These methods prepare, execute, and monitor the RTC6 command lists.
    void OpenList(UINT listNo);        // Prepares a list for writing commands
    void CloseList();                  // Marks the end of commands in a list
    void ExecuteList(UINT listNo);     // Starts execution of a list
    bool IsListBusy() const;           // Checks if a list is currently busy (executing)
    UINT GetListCurrentPointer() const; // Retrieves the current instruction pointer in the list

    // --- Access to Injected Components (for higher-level layers like MicroVectorProcessor) ---
    // These getters return non-owning references to the owned interfaces.
    // This allows other classes (e.g., MicroVectorProcessor) to use these services.
    IListCommandBuilder* GetListCommandBuilder() const { return commandBuilder_.get(); }
    ILaserControl* GetLaserControl() const { return laserControl_.get(); }

private:
    // Owned interfaces (unique_ptrs manage their lifetime)
    std::unique_ptr<IRTCAPIDriver> driver_;
    std::unique_ptr<IListCommandBuilder> commandBuilder_;
    std::unique_ptr<ILaserControl> laserControl_;

    bool isInitialized_ = false; // Internal state: track if system is fully initialized
    UINT activeBoardId_ = 0;     // Store the ID of the actively selected board
};

#endif // RTC6_CONTROLLER_H