// =========================================================================
// ===               File: GeometryHandler_LogicTest.cpp                 ===
// =========================================================================
#include "pch.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GeometryHandler.h"
#include "InterfaceListHandler.h"
#include "MachineConfig.h" 
#include "MockListHandler.h"

class GeometryHandler_LogicTest : public ::testing::Test {
protected:
    void SetUp() override {
        handler = std::make_unique<GeometryHandler>(dummyMock);
    }

    int callMmToBits(double mm) {
        return handler->mmToBits(mm);
    }

    MockListHandler dummyMock;
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