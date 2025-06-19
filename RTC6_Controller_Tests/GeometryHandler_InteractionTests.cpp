// =========================================================================
// ===             File: GeometryHandler_InteractionTest.cpp             ===
// =========================================================================
#pragma once
#include "pch.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GeometryHandler.h"
#include "InterfaceListHandler.h"
#include "open_vector_format.pb.h"
#include "MachineConfig.h"
#include <cmath>

using ::testing::_;
using ::testing::InSequence;
using ::testing::DoubleEq;

MATCHER_P(IsCloseToInt, expected, "") {
    *result_listener << "where the value " << arg << " is compared to " << expected;
    return std::abs(static_cast<double>(arg) - expected) <= 1.0;
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

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_WithLineSequence_CallsListHandlerWithCorrectValues) {
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

    const double factor = MachineConfig::MM_TO_BITS_CONVERSION_FACTOR;
    const double power_percent = (params.laser_power_in_w() / MachineConfig::MAX_LASER_POWER_W) * 100.0;
    const double expected_dac = (power_percent / 100.0) * 4095.0;
    const double expected_focus_bits = -2.5 * factor;

    EXPECT_CALL(mockListHandler, addSetMarkSpeed(DoubleEq(1000.0)));
    EXPECT_CALL(mockListHandler, addSetLaserPower(1, IsCloseToInt(expected_dac)));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(IsCloseToInt(expected_focus_bits)));

    {
        InSequence s;
        EXPECT_CALL(mockListHandler, addJumpAbsolute(IsCloseToInt(10.0 * factor), IsCloseToInt(20.0 * factor)));
        EXPECT_CALL(mockListHandler, addMarkAbsolute(IsCloseToInt(30.0 * factor), IsCloseToInt(40.0 * factor)));
        EXPECT_CALL(mockListHandler, addMarkAbsolute(IsCloseToInt(50.0 * factor), IsCloseToInt(60.0 * factor)));
    }

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_WithHatches_CallsListHandlerWithCorrectJumpMarkPairs) {
    open_vector_format::VectorBlock block;

    auto* hatches = block.mutable__hatches();

    // Hatch 1: (1,1) -> (10,1)
    hatches->add_points(1.0f); hatches->add_points(1.0f);
    hatches->add_points(10.0f); hatches->add_points(1.0f);
    // Hatch 2: (1,2) -> (10,2)
    hatches->add_points(1.0f); hatches->add_points(2.0f);
    hatches->add_points(10.0f); hatches->add_points(2.0f);

    open_vector_format::MarkingParams params;

    const double factor = MachineConfig::MM_TO_BITS_CONVERSION_FACTOR;

    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_));
    EXPECT_CALL(mockListHandler, addSetLaserPower(_, _));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_));

    {
        InSequence s;
        // Hatch 1
        EXPECT_CALL(mockListHandler, addJumpAbsolute(IsCloseToInt(1.0 * factor), IsCloseToInt(1.0 * factor)));
        EXPECT_CALL(mockListHandler, addMarkAbsolute(IsCloseToInt(10.0 * factor), IsCloseToInt(1.0 * factor)));
        // Hatch 2
        EXPECT_CALL(mockListHandler, addJumpAbsolute(IsCloseToInt(1.0 * factor), IsCloseToInt(2.0 * factor)));
        EXPECT_CALL(mockListHandler, addMarkAbsolute(IsCloseToInt(10.0 * factor), IsCloseToInt(2.0 * factor)));
    }

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_WithUnsupportedType_SetsParamsButMakesNoGeometryCalls) {
    // Arrange
    open_vector_format::VectorBlock block;
    block.mutable_point_sequence();
    open_vector_format::MarkingParams params;

    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_)).Times(1);
    EXPECT_CALL(mockListHandler, addSetLaserPower(_, _)).Times(1);
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_)).Times(1);

    EXPECT_CALL(mockListHandler, addJumpAbsolute(_, _)).Times(0);
    EXPECT_CALL(mockListHandler, addMarkAbsolute(_, _)).Times(0);

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_WithSingleLineSegment_MakesOneJumpOneMark) {
    // Arrange: A simple line from (1,2) to (3,4)
    open_vector_format::VectorBlock block;
    auto* line_seq = block.mutable_line_sequence();
    line_seq->add_points(1.0f); line_seq->add_points(2.0f);
    line_seq->add_points(3.0f); line_seq->add_points(4.0f);
    open_vector_format::MarkingParams params;
    const double factor = MachineConfig::MM_TO_BITS_CONVERSION_FACTOR;

    // Expect: Parameter setting calls, then one jump and one mark.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_));
    EXPECT_CALL(mockListHandler, addSetLaserPower(_, _));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_));
    {
        InSequence s;
        EXPECT_CALL(mockListHandler, addJumpAbsolute(IsCloseToInt(1.0 * factor), IsCloseToInt(2.0 * factor)));
        EXPECT_CALL(mockListHandler, addMarkAbsolute(IsCloseToInt(3.0 * factor), IsCloseToInt(4.0 * factor)));
    }

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_WithSingleHatch_MakesOneJumpOneMark) {
    // Arrange: A single hatch from (5,6) to (7,8)
    open_vector_format::VectorBlock block;
    auto* hatches = block.mutable__hatches();
    hatches->add_points(5.0f); hatches->add_points(6.0f);
    hatches->add_points(7.0f); hatches->add_points(8.0f);
    open_vector_format::MarkingParams params;
    const double factor = MachineConfig::MM_TO_BITS_CONVERSION_FACTOR;

    // Expect: Parameter setting calls, then one jump and one mark.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_));
    EXPECT_CALL(mockListHandler, addSetLaserPower(_, _));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_));
    {
        InSequence s;
        EXPECT_CALL(mockListHandler, addJumpAbsolute(IsCloseToInt(5.0 * factor), IsCloseToInt(6.0 * factor)));
        EXPECT_CALL(mockListHandler, addMarkAbsolute(IsCloseToInt(7.0 * factor), IsCloseToInt(8.0 * factor)));
    }

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_WithEmptyVectorData_MakesNoGeometryCalls) {
    // Arrange: A block with no geometry type set (VECTOR_DATA_NOT_SET)
    open_vector_format::VectorBlock block;
    open_vector_format::MarkingParams params;

    // Expect: It should still set the parameters for the block.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_)).Times(1);
    EXPECT_CALL(mockListHandler, addSetLaserPower(_, _)).Times(1);
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_)).Times(1);

    // Expect: It should NOT make any geometry calls (jump or mark).
    EXPECT_CALL(mockListHandler, addJumpAbsolute(_, _)).Times(0);
    EXPECT_CALL(mockListHandler, addMarkAbsolute(_, _)).Times(0);

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_LineSequenceWithInsufficientPoints_MakesNoGeometryCalls) {
    // Arrange: A line sequence with only one point (2 floats), which is not enough for a line.
    open_vector_format::VectorBlock block;
    auto* line_seq = block.mutable_line_sequence();
    line_seq->add_points(1.0f); line_seq->add_points(2.0f);
    open_vector_format::MarkingParams params;

    // Expect: Parameters are set, but no geometry calls are made due to the size guard.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_)).Times(1);
    EXPECT_CALL(mockListHandler, addSetLaserPower(_, _)).Times(1);
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_)).Times(1);
    EXPECT_CALL(mockListHandler, addJumpAbsolute(_, _)).Times(0);
    EXPECT_CALL(mockListHandler, addMarkAbsolute(_, _)).Times(0);

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_HatchesWithInsufficientPoints_MakesNoGeometryCalls) {
    // Arrange: Hatches with only one point (2 floats), which is not enough for a hatch.
    open_vector_format::VectorBlock block;
    auto* hatches = block.mutable__hatches();
    hatches->add_points(1.0f); hatches->add_points(2.0f);
    open_vector_format::MarkingParams params;

    // Expect: Parameters are set, but no geometry calls are made due to the size guard.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_)).Times(1);
    EXPECT_CALL(mockListHandler, addSetLaserPower(_, _)).Times(1);
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_)).Times(1);
    EXPECT_CALL(mockListHandler, addJumpAbsolute(_, _)).Times(0);
    EXPECT_CALL(mockListHandler, addMarkAbsolute(_, _)).Times(0);

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_ZeroPowerParameter_CorrectlySetsZeroDac) {
    // Arrange
    open_vector_format::VectorBlock block;
    block.mutable_line_sequence()->add_points(0); // Dummy geometry to trigger processing
    block.mutable_line_sequence()->add_points(0);
    block.mutable_line_sequence()->add_points(1);
    block.mutable_line_sequence()->add_points(1);

    open_vector_format::MarkingParams params;
    params.set_laser_power_in_w(0.0); // Test the boundary condition of 0W power.

    // Expect: The power setting call should have a DAC value of 0.
    EXPECT_CALL(mockListHandler, addSetLaserPower(1, IsCloseToInt(0.0)));
    // We don't care about the other calls for this specific test.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_));
    EXPECT_CALL(mockListHandler, addJumpAbsolute(_, _));
    EXPECT_CALL(mockListHandler, addMarkAbsolute(_, _));

    // Act
    handler->processVectorBlock(block, params);
}

TEST_F(GeometryHandler_InteractionTest, ProcessVectorBlock_MaxPowerParameter_CorrectlySetsMaxDac) {
    // Arrange
    open_vector_format::VectorBlock block;
    block.mutable_line_sequence()->add_points(0); // Dummy geometry
    block.mutable_line_sequence()->add_points(0);
    block.mutable_line_sequence()->add_points(1);
    block.mutable_line_sequence()->add_points(1);

    open_vector_format::MarkingParams params;
    // Test the boundary condition of exactly maximum power.
    params.set_laser_power_in_w(MachineConfig::MAX_LASER_POWER_W);

    // Expect: The power setting call should have the max DAC value of 4095.
    EXPECT_CALL(mockListHandler, addSetLaserPower(1, IsCloseToInt(4095.0)));
    // We don't care about the other calls for this specific test.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(_));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(_));
    EXPECT_CALL(mockListHandler, addJumpAbsolute(_, _));
    EXPECT_CALL(mockListHandler, addMarkAbsolute(_, _));

    // Act
    handler->processVectorBlock(block, params);
}