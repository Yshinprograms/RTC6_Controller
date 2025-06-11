// GeometryHandler_InteractionTests.cpp
#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "GeometryHandler.h"
#include "InterfaceListHandler.h"
#include "Geometry.h"

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


TEST_F(GeometryHandler_InteractionTest, ProcessPolyline_WithValidInput_CallsListHandlerInCorrectSequence) {
    const std::vector<Point> polyline = { {10.0, 20.0}, {30.0, 40.0}, {50.0, 60.0} };
    const double power = 50.0;
    const double speed = 1000.0;
    const double focus = -2.5;

    const UINT expected_dac_value = 2047;    // 50% of 4095, truncated
    const INT expected_focus_bits = -2500;   // -2.5mm * 1000.0 BITS_PER_MM

    ::testing::InSequence s;

    EXPECT_CALL(mockListHandler, addSetMarkSpeed(speed));
    EXPECT_CALL(mockListHandler, addSetLaserPower(1, expected_dac_value));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(expected_focus_bits));
    EXPECT_CALL(mockListHandler, addJumpAbsolute(10000, 20000));
    EXPECT_CALL(mockListHandler, addMarkAbsolute(30000, 40000));
    EXPECT_CALL(mockListHandler, addMarkAbsolute(50000, 60000));

    handler->processPolyline(polyline, power, speed, focus);
}

TEST_F(GeometryHandler_InteractionTest, ProcessPolyline_WithFewerThanTwoPoints_MakesNoListCalls) {
    const std::vector<Point> emptyPolyline = {};
    const std::vector<Point> singlePointPolyline = { {10, 10} };

    EXPECT_CALL(mockListHandler, addJumpAbsolute(::testing::_, ::testing::_)).Times(0);
    EXPECT_CALL(mockListHandler, addMarkAbsolute(::testing::_, ::testing::_)).Times(0);
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(::testing::_)).Times(0);

    handler->processPolyline(emptyPolyline, 50.0, 1000.0, 0.0);
    handler->processPolyline(singlePointPolyline, 50.0, 1000.0, 0.0);
}

TEST_F(GeometryHandler_InteractionTest, ProcessPolyline_WithDecimalCoordinates_CorrectlyConvertsToBitsForListCalls) {
    const std::vector<Point> polyline = { {0.0, 0.0}, {12.345, -67.891} };

    ::testing::InSequence s;

    // Expect prerequisite parameter calls; their values are not the focus of this test.
    EXPECT_CALL(mockListHandler, addSetMarkSpeed(::testing::_));
    EXPECT_CALL(mockListHandler, addSetLaserPower(::testing::_, ::testing::_));
    EXPECT_CALL(mockListHandler, addSetFocusOffset(::testing::_));

    EXPECT_CALL(mockListHandler, addJumpAbsolute(0, 0));
    EXPECT_CALL(mockListHandler, addMarkAbsolute(12345, -67891));

    handler->processPolyline(polyline, 50.0, 1000.0, 0.0);
}