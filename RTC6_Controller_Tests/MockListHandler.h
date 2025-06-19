#pragma once
#include "gmock/gmock.h"
#include "InterfaceListHandler.h"

class MockListHandler : public InterfaceListHandler {
public:
    MOCK_METHOD(bool, setupAutoChangeMode, (), (override));
    MOCK_METHOD(void, reArmAutoChange, (), (override));
    MOCK_METHOD(bool, beginListPreparation, (), (override));
    MOCK_METHOD(void, endListPreparation, (), (override));
    MOCK_METHOD(bool, executeCurrentListAndCycle, (), (override));
    MOCK_METHOD(bool, isListBusy, (UINT listIdToCheck), (const, override));
    MOCK_METHOD(UINT, getCurrentFillListId, (), (const, override));
    MOCK_METHOD(void, addJumpAbsolute, (INT x, INT y), (override));
    MOCK_METHOD(void, addMarkAbsolute, (INT x, INT y), (override));
    MOCK_METHOD(void, addSetFocusOffset, (INT offset_bits), (override));
    MOCK_METHOD(void, addSetMarkSpeed, (double speed_mm_s), (override));
    MOCK_METHOD(void, addSetLaserPower, (UINT port, UINT power), (override));
	MOCK_METHOD(UINT, getLastExecutedListId, (), (const, override));
};