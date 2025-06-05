// Implementations/RTC6Controller.cpp
#include "RTC6Controller.h"
#include <iostream> // For logging/status messages
#include <bitset>   // For debugging accumulated errors

// Constructor: Receives unique_ptrs, taking ownership of the injected dependencies.
RTC6Controller::RTC6Controller(std::unique_ptr<IRTCAPIDriver> driver,
    std::unique_ptr<IListCommandBuilder> commandBuilder,
    std::unique_ptr<ILaserControl> laserControl)
    : driver_(std::move(driver)),
    commandBuilder_(std::move(commandBuilder)),
    laserControl_(std::move(laserControl)) {
    // Validate that all essential dependencies are provided.
    if (!driver_ || !commandBuilder_ || !laserControl_) {
        throw std::invalid_argument("RTC6Controller: All injected interface pointers must be non-null.");
    }
    std::cout << "[RTC6Controller] Instance created, dependencies injected." << std::endl;
}

// Destructor (defaulted, unique_ptrs handle cleanup of owned objects).
RTC6Controller::~RTC6Controller() {
    std::cout << "[RTC6Controller] Instance destroyed." << std::endl;
}

// --- High-Level System Control & Initialization Implementations ---

// Orchestrates the full initialization sequence: DLL, board selection, firmware load.
void RTC6Controller::InitializeAndLoadFirmware(UINT boardId) {
    if (isInitialized_) {
        std::cout << "[RTC6Controller] Already initialized. Skipping initialization." << std::endl;
        return;
    }
    std::cout << "\n[RTC6Controller] Performing full system initialization sequence..." << std::endl;

    // 1. Initialize RTC6 DLL
    driver_->init_rtc6_dll();
    std::cout << "[RTC6Controller] RTC6 DLL initialized successfully." << std::endl;

    // 2. Count detected boards and select the target board
    UINT boardCount = driver_->rtc6_count_cards();
    std::cout << "[RTC6Controller] Detected " << boardCount << " RTC6 board(s)." << std::endl;
    if (boardCount == 0) {
        throw Rtc6Exception("No RTC6 boards detected. Cannot proceed.", RTC6::ErrorCode::HARDWARE_ERROR);
    }

    // Ensure the requested boardId is within detected range
    if (boardId == 0 || boardId > boardCount) {
        std::string msg = "Requested board ID " + std::to_string(boardId) +
            " is out of range. Detected: " + std::to_string(boardCount);
        throw Rtc6Exception(msg, RTC6::ErrorCode::INVALID_PARAMETER);
    }

    std::cout << "[RTC6Controller] Selecting RTC6 board " << boardId << "..." << std::endl;
    driver_->select_rtc(boardId); // Selects the board
    activeBoardId_ = boardId; // Store the ID of the successfully selected board
    std::cout << "[RTC6Controller] RTC6 board " << activeBoardId_ << " selected successfully." << std::endl;

    // 3. Load DSP firmware (assuming files are in the executable directory or default path 0)
    std::cout << "[RTC6Controller] Loading RTC6 DSP firmware (path 0)..." << std::endl;
    driver_->load_program_file(0);
    std::cout << "[RTC6Controller] DSP program file loaded successfully." << std::endl;

    // 4. Reset any accumulated errors after initialization.
    driver_->reset_error(activeBoardId_);
    isInitialized_ = true;
    std::cout << "[RTC6Controller] System initialization complete for board " << activeBoardId_ << "." << std::endl;
}

// Queries and retrieves various board identification and version information.
void RTC6Controller::QueryBoardInformation(UINT boardId, UINT& rtcVersion, UINT& biosVersion, UINT& serialNumber) {
    if (!isInitialized_ || activeBoardId_ != boardId) {
        throw Rtc6Exception("RTC6Controller not initialized or board ID mismatch. Call InitializeAndLoadFirmware first.", RTC6::ErrorCode::NOT_INITIALIZED);
    }
    std::cout << "\n[RTC6Controller] Retrieving board information for board " << boardId << "..." << std::endl;

    rtcVersion = driver_->get_rtc_version();
    biosVersion = driver_->get_bios_version();
    serialNumber = driver_->n_get_serial_number(boardId); // Use n_get_serial_number for specific board ID

    // Check for any accumulated errors that might have occurred during queries.
    UINT32 accumulatedError = driver_->get_error();
    if (accumulatedError != 0) {
        std::cerr << "WARNING: Accumulated errors on board " << boardId << ". Code: 0x"
            << std::hex << accumulatedError << std::dec << " (" << std::bitset<32>(accumulatedError) << ")" << std::endl;
        driver_->reset_error(boardId); // Clear after reporting
    }
    std::cout << "[RTC6Controller] Board information retrieved." << std::endl;
}

// Configures global laser parameters (mode, control signals, default speeds).
void RTC6Controller::ConfigureGlobalLaser(UINT mode, UINT controlSignals, double markSpeed, double jumpSpeed) {
    if (!isInitialized_) {
        throw Rtc6Exception("RTC6Controller not initialized. Cannot configure laser.", RTC6::ErrorCode::NOT_INITIALIZED);
    }
    std::cout << "[RTC6Controller] Configuring global laser parameters..." << std::endl;
    // Delegate to the injected LaserControl service
    laserControl_->ConfigureGlobalLaser(mode, controlSignals, markSpeed, jumpSpeed);
    std::cout << "[RTC6Controller] Global laser parameters configured." << std::endl;
}

// --- High-Level List Management Implementations ---

// Prepares a specific list number for writing commands.
void RTC6Controller::OpenList(UINT listNo) {
    if (!isInitialized_) {
        throw Rtc6Exception("RTC6Controller not initialized. Cannot open list.", RTC6::ErrorCode::NOT_INITIALIZED);
    }
    std::cout << "[RTC6Controller] Opening list " << listNo << " for writing..." << std::endl;
    commandBuilder_->SetStartList(listNo);
}

// Marks the end of the current list, indicating it's ready for execution.
void RTC6Controller::CloseList() {
    if (!isInitialized_) {
        throw Rtc6Exception("RTC6Controller not initialized. Cannot close list.", RTC6::ErrorCode::NOT_INITIALIZED);
    }
    std::cout << "[RTC6Controller] Closing current list with SetEndOfList()..." << std::endl;
    commandBuilder_->SetEndOfList();
}

// Starts the execution of a previously prepared list.
void RTC6Controller::ExecuteList(UINT listNo) {
    if (!isInitialized_) {
        throw Rtc6Exception("RTC6Controller not initialized. Cannot execute list.", RTC6::ErrorCode::NOT_INITIALIZED);
    }
    std::cout << "[RTC6Controller] Executing list " << listNo << "..." << std::endl;
    commandBuilder_->ExecuteList(listNo);
}

// Checks if the RTC6 board's DSP is currently busy executing a list.
bool RTC6Controller::IsListBusy() const {
    if (!isInitialized_) {
        throw Rtc6Exception("RTC6Controller not initialized. Cannot check list status.", RTC6::ErrorCode::NOT_INITIALIZED);
    }
    return (driver_->get_status() & 0x00000002) != 0; // Check BUSY bit (Bit 1) of get_status
}

// Retrieves the current instruction pointer of the list being executed by the DSP.
UINT RTC6Controller::GetListCurrentPointer() const {
    if (!isInitialized_) {
        throw Rtc6Exception("RTC6Controller not initialized. Cannot get list pointer.", RTC6::ErrorCode::NOT_INITIALIZED);
    }
    return driver_->get_list_pointer();
}