// Implementations/ListCommandBuilder.h
#ifndef LIST_COMMAND_BUILDER_H
#define LIST_COMMAND_BUILDER_H

#include "IListCommandBuilder.h" // Inherit from the interface
#include "IRTCAPIDriver.h"      // For the injected driver dependency
#include <stdexcept>                          // For std::invalid_argument

// Concrete implementation of IListCommandBuilder.
// It uses an IRTCAPIDriver to send the actual commands to the RTC6 board.
class ListCommandBuilder : public IListCommandBuilder {
public:
    // Constructor: Takes a non-owning pointer to the low-level driver.
    explicit ListCommandBuilder(IRTCAPIDriver* driver);

    // Destructor (defaulted as it owns no resources that need explicit cleanup).
    ~ListCommandBuilder() override = default;

    // Prevent copying and assignment
    ListCommandBuilder(const ListCommandBuilder&) = delete;
    ListCommandBuilder& operator=(const ListCommandBuilder&) = delete;

    // --- Implementations of IListCommandBuilder methods ---

    // List Control
    void SetStartList(UINT listNumber) override;
    void SetEndOfList() override;
    void ExecuteList(UINT listNumber) override;

    // Basic Motion Commands
    void JumpAbsolute(LONG x, LONG y) override;
    void MarkAbsolute(LONG x, LONG y) override;

    // Micro-vector Command
    void AddMicroVector(LONG x, LONG y, LONG laserOnBits, LONG laserOffBits) override;

    // Dynamic Parameter Control (List-based)
    void SetCurrentListLaserPower(UINT powerValue) override;
    void SetCurrentListMarkSpeed(double speedValue) override;
    void SetCurrentListZAxisHeight(LONG zAxisValue) override;

private:
    IRTCAPIDriver* driver_; // Non-owning pointer to the low-level RTC6 API driver.
};

#endif // LIST_COMMAND_BUILDER_H