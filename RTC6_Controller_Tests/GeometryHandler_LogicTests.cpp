#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GeometryHandler.h"
#include "InterfaceListHandler.h"
#include "Geometry.h"

// A dummy mock to satisfy the constructor. It will never be used.
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

class GeometryHandler_PrivateMethodsTest : public ::testing::Test {
protected:
    void SetUp() override {
        handler = std::make_unique<GeometryHandler>(dummyMock);
    }

    int callMmToBits(double mm) {
        return handler->mmToBits(mm);
    }

    UINT callPowerToDAC(double percent) {
        return handler->powerToDAC(percent);
    }

    static bool IsPowerConversionCorrect(double percent) {
        DummyMockListHandler dummy;
        GeometryHandler handler(dummy);
        UINT actualDac = handler.powerToDAC(percent);
        UINT expectedDac = static_cast<UINT>((percent / 100.0) * 4095.0);
        return actualDac == expectedDac;
    }

    DummyMockListHandler dummyMock;
    std::unique_ptr<GeometryHandler> handler;
};

// Do NOT directly call handler->method within the Macros
// May not work correctly, hence use the helper functions!
TEST_F(GeometryHandler_PrivateMethodsTest, MmToBits_WithIntegerInput_ReturnsCorrectBitValue) {
    ASSERT_EQ(callMmToBits(10.0), 10000);
    ASSERT_EQ(callMmToBits(-5.0), -5000);
    ASSERT_EQ(callMmToBits(0.0), 0);
}

TEST_F(GeometryHandler_PrivateMethodsTest, MmToBits_WithDecimalInput_CorrectlyRoundsResult) {
    ASSERT_EQ(callMmToBits(12.3456), 12346); // Rounds up
    ASSERT_EQ(callMmToBits(67.8912), 67891); // Rounds down
    ASSERT_EQ(callMmToBits(99.9995), 100000); // Rounds at the .5 boundary
}

TEST_F(GeometryHandler_PrivateMethodsTest, PowerToDac_WithOutOfRangeInput_IsClamped) {
    // A value below zero should be clamped to 0.
    EXPECT_EQ(callPowerToDAC(-25.0), 0);
    EXPECT_EQ(callPowerToDAC(-0.001), 0);
    // A value above 100 should be clamped to the maximum DAC value (4095).
    EXPECT_EQ(callPowerToDAC(150.0), 4095);
    EXPECT_EQ(callPowerToDAC(100.001), 4095);
}

TEST_F(GeometryHandler_PrivateMethodsTest, PowerToDac_WithBoundaryInputs_ReturnsCorrectMinMaxValues) {
    EXPECT_EQ(callPowerToDAC(0.0), 0);
    EXPECT_EQ(callPowerToDAC(100.0), 4095);
}