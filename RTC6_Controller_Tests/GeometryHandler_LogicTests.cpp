// =========================================================================
// ===               File: GeometryHandler_LogicTest.cpp                 ===
// =========================================================================
#include "pch.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GeometryHandler.h"
#include "InterfaceListHandler.h"
#include "ProcessData.h"
#include "MachineConfig.h" 

class DummyMockListHandler : public InterfaceListHandler {
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
};

class GeometryHandler_LogicTest : public ::testing::Test {
protected:
    void SetUp() override {
        handler = std::make_unique<GeometryHandler>(dummyMock);
    }

    int callMmToBits(double mm) {
        return handler->mmToBits(mm);
    }

    DummyMockListHandler dummyMock;
    std::unique_ptr<GeometryHandler> handler;
};

TEST_F(GeometryHandler_LogicTest, MmToBits_WithIntegerInput_ReturnsCorrectBitValue) {
    const int expected_bits_10mm = static_cast<int>(std::round(10.0 * MachineConfig::MM_TO_BITS_CONVERSION_FACTOR));
    const int expected_bits_neg_5mm = static_cast<int>(std::round(-5.0 * MachineConfig::MM_TO_BITS_CONVERSION_FACTOR));

    ASSERT_EQ(callMmToBits(10.0), expected_bits_10mm);
    ASSERT_EQ(callMmToBits(-5.0), expected_bits_neg_5mm);
    ASSERT_EQ(callMmToBits(0.0), 0);
}

TEST_F(GeometryHandler_LogicTest, MmToBits_WithDecimalInput_CorrectlyRoundsResult) {
    // This test verifies the rounding behavior by comparing against std::round.
    const double factor = MachineConfig::MM_TO_BITS_CONVERSION_FACTOR;
    ASSERT_EQ(callMmToBits(12.3456), static_cast<int>(std::round(12.3456 * factor)));
    ASSERT_EQ(callMmToBits(67.8912), static_cast<int>(std::round(67.8912 * factor)));
    ASSERT_EQ(callMmToBits(99.9995), static_cast<int>(std::round(99.9995 * factor)));
    ASSERT_EQ(callMmToBits(-12.3456), static_cast<int>(std::round(-12.3456 * factor)));
}