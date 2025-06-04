#pragma once
#include "../Interfaces/IListCommandBuilder.h"

// Forward declaration
class IRTCAPIDriver;

class ListCommandBuilder : public IListCommandBuilder {
public:
    explicit ListCommandBuilder(IRTCAPIDriver* driver);
    ~ListCommandBuilder() override = default;

    // List control
    void SetStartList(int listNumber) override;
    void SetEndOfList() override;
    void ExecuteList(int listNumber) override;
    int GetListStatus(int listNumber) const override;
    int GetListPointer() const override;

    // Basic motion commands
    void JumpAbsolute(int x, int y) override;
    void MarkAbsolute(int x, int y) override;

    // Micro-vector commands
    void AddMicroVector(int x, int y, int dt) override;
    void AddLaserSignalOnList() override;
    void AddLaserSignalOffList() override;
    void SetCurrentListLaserPower(UINT powerValue) override;
    void SetCurrentListMarkSpeed(double speedValue) override;
    void SetCurrentListZAxisHeight(int zAxisValue) override;

private:
    IRTCAPIDriver* m_driver;  // Non-owning pointer to the RTC6 API driver
};
