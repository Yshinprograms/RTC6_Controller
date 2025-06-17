#pragma once
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GeometryHandler.h"
#include "InterfaceListHandler.h"
#include "open_vector_format.pb.h"
#include <cmath>

using ::testing::_;
using ::testing::InSequence;
using ::testing::DoubleEq;

MATCHER_P(IsCloseToInt, expected, "") {
    return std::abs(static_cast<double>(arg) - static_cast<double>(expected)) <= 1.0;
}

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
};

class GeometryHandler_InteractionTest : public ::testing::Test {
protected:
    void SetUp() override {
        handler = std::make_unique<GeometryHandler>(mockListHandler);
    }
    MockListHandler mockListHandler;
    std::unique_ptr<GeometryHandler> handler;
};

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_WithValidLineSequence_CallsListHandlerInCorrectSequence) {
    // Arrange
    open_vector_format::VectorBlock block;
    auto* line_seq = block.mutable_line_sequence();
    line_seq->add_points(10.0f); line_seq->add_points(20.0f);
    line_seq->add_points(30.0f); line_seq->add_points(40.0f);
    line_seq->add_points(50.0f); line_seq->add_points(60.0f);

    open_vector_format::MarkingParams params;
    params.set_laser_power_in_w(50.0);
    params.set_laser_speed_in_mm_per_s(1000.0);
    params.set_laser_focus_shift_in_mm(-2.5);

    const UINT expected_dac_value = 2047;
    const INT expected_focus_bits = -2500;

    // Expect: Parameters can be set in any order.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(DoubleEq(1000.0)));
    EXPECT_CALL(mockListHandler, addSetLaserPower(1, IsCloseToInt(expected_dac_value)));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(IsCloseToInt(expected_focus_bits)));

    // FIX: Only enforce the order of geometry commands.
    {
        InSequence s;
        EXPECT_CALL(mockListHandler, addJumpAbsolute(IsCloseToInt(10000), IsCloseToInt(20000)));
        EXPECT_CALL(mockListHandler, addMarkAbsolute(IsCloseToInt(30000), IsCloseToInt(40000)));
        EXPECT_CALL(mockListHandler, addMarkAbsolute(IsCloseToInt(50000), IsCloseToInt(60000)));
    }

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_WithEmptyLineSequence_MakesNoGeometryCalls) {
    // Arrange
    open_vector_format::VectorBlock block;
    block.mutable_line_sequence();
    open_vector_format::MarkingParams params;

    // Expect
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_)).Times(1);
    EXPECT_CALL(mockListHandler, addSetLaserPower(_, _)).Times(1);
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_)).Times(1);
    EXPECT_CALL(mockListHandler, addJumpAbsolute(_, _)).Times(0);
    EXPECT_CALL(mockListHandler, addMarkAbsolute(_, _)).Times(0);

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_WithDecimalCoordinates_CorrectlyConvertsToBits) {
    // Arrange
    open_vector_format::VectorBlock block;
    auto* line_seq = block.mutable_line_sequence();
    line_seq->add_points(0.0f);     line_seq->add_points(0.0f);
    line_seq->add_points(12.345f);  line_seq->add_points(-67.891f);

    open_vector_format::MarkingParams params;

    // Expect: Parameter calls can happen in any order.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_));
    EXPECT_CALL(mockListHandler, addSetLaserPower(_, _));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_));

    // FIX: Only enforce the order of geometry commands.
    {
        InSequence s;
        EXPECT_CALL(mockListHandler, addJumpAbsolute(IsCloseToInt(0), IsCloseToInt(0)));
        EXPECT_CALL(mockListHandler, addMarkAbsolute(IsCloseToInt(12345), IsCloseToInt(-67891)));
    }

    // Act
    handler->processVectorBlock(block, params);
}