// ListHandler_InteractionTests.cpp
#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "InterfaceCommunicator.h"
#include "InterfaceListHandler.h"
#include "ListHandler.h"

class MockCommunicator : public InterfaceCommunicator {
public:
	MOCK_METHOD(bool, isSuccessfullySetup, (), (const, override));
};

class ListHandler_InteractionTest : public ::testing::Test {
protected:
	void SetUp() override {
		handler = std::make_unique<ListHandler>(mockCommunicator);
	}

	void callSwitchFillListTarget() {
		return handler->switchFillListTarget();
	}

	int callMmToBits(double mm) const {
		return handler->mmToBits(mm);
	}

	MockCommunicator mockCommunicator;
	std::unique_ptr<ListHandler> handler;
};

TEST_F(ListHandler_InteractionTest, SetupAutoChangeMode_WhenCommunicatorIsReady_ReturnsTrue) {
	EXPECT_CALL(mockCommunicator, isSuccessfullySetup())
		.Times(1)
		.WillOnce(::testing::Return(true));

	bool result = handler->setupAutoChangeMode();
	ASSERT_TRUE(result);
}

TEST_F(ListHandler_InteractionTest, SetupAutoChangeMode_WhenCommunicatorNotSetup_ReturnsFalse) {
	EXPECT_CALL(mockCommunicator, isSuccessfullySetup())
		.Times(1)
		.WillOnce(::testing::Return(false));

	bool result = handler->setupAutoChangeMode();
	ASSERT_FALSE(false);
}