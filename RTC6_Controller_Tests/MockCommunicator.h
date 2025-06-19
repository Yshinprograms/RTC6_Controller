#pragma once
#include "gmock/gmock.h"
#include "InterfaceCommunicator.h"

/**
 * @class MockCommunicator
 * @brief A Google Mock (GMock) implementation of the InterfaceCommunicator.
 *
 * This class is used exclusively for testing purposes. It allows us to create
 * a "fake" communicator object that we can control within our unit tests.
 * This is essential for isolating the class under test from the actual
 * Rtc6Communicator, so we can test its logic without needing a real hardware
 * connection.
 */
class MockCommunicator : public InterfaceCommunicator {
public:
	MOCK_METHOD(bool, isSuccessfullySetup, (), (const, override));
	MOCK_METHOD(bool, connectAndSetupBoard, (), (override));
};