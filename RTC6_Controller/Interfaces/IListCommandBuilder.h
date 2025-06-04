#ifndef I_LIST_COMMAND_BUILDER_H
#define I_LIST_COMMAND_BUILDER_H

#include "Rtc6Common.h"

/**
 * @brief Interface for building RTC6 command lists with micro-vector support
 */
class IListCommandBuilder {
public:
    virtual ~IListCommandBuilder() = default;

    // List control
    virtual void SetStartList(int listNumber) = 0;
    virtual void SetEndOfList() = 0;
    virtual void ExecuteList(int listNumber) = 0;
    virtual int GetListStatus(int listNumber) const = 0;
    virtual int GetListPointer() const = 0;

    // Basic motion commands
    virtual void JumpAbsolute(int x, int y) = 0;
    virtual void MarkAbsolute(int x, int y) = 0;

    // Micro-vector commands
    virtual void AddMicroVector(int x, int y, int dt) = 0;
    virtual void AddLaserSignalOnList() = 0;
    virtual void AddLaserSignalOffList() = 0;
    virtual void SetCurrentListLaserPower(UINT powerValue) = 0;
    virtual void SetCurrentListMarkSpeed(double speedValue) = 0;
    virtual void SetCurrentListZAxisHeight(int zAxisValue) = 0;
};

#endif // I_LIST_COMMAND_BUILDER_H
