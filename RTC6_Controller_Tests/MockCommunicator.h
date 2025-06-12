#pragma once
#include "gmock/gmock.h"
#include "InterfaceCommunicator.h" // The "contract" this mock must fulfill.

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
    // MOCK_METHOD generates a mock implementation for the virtual function
    // from the InterfaceCommunicator base class.
    //
    // Parameters:
    // 1. Return Type: bool
    // 2. Method Name: isSuccessfullySetup
    // 3. Arguments: () - none
    // 4. Qualifiers: (const, override) - It's a const method and overrides a base class virtual function.
    MOCK_METHOD(bool, isSuccessfullySetup, (), (const, override));
};