#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ListHandler.h"
#include "MockCommunicator.h"
#include "MockRtcApi.h" // <-- Include the new mock
#include <memory>

using ::testing::Return;
using ::testing::_;

// Test Fixture for ListHandler Logic/State tests
class ListHandler_LogicTest : public ::testing::Test {
protected:
    // We now need mocks for BOTH dependencies.
    std::unique_ptr<MockCommunicator> mockCommunicator;
    std::unique_ptr<MockRtcApi> mockRtcApi; // <-- Add the new mock
    std::unique_ptr<ListHandler> listHandler;

    void SetUp() override {
        // 1. Create the mock dependencies
        mockCommunicator = std::make_unique<MockCommunicator>();
        mockRtcApi = std::make_unique<MockRtcApi>(); // <-- Create an instance of the new mock

        // 2. Set default behavior for the mocks.
        // We still need the communicator to report it's ready.
        ON_CALL(*mockCommunicator, isSuccessfullySetup()).WillByDefault(Return(true));

        // For logic tests, we don't care about the API calls, but the code will
        // still call them. We can add default actions here to prevent GMock warnings
        // about "uninteresting calls".
        ON_CALL(*mockRtcApi, api_execute_list(_)).WillByDefault(Return());

        // 3. Create the REAL object under test, injecting BOTH mocks.
        listHandler = std::make_unique<ListHandler>(*mockCommunicator, *mockRtcApi);
    }

    void TearDown() override {
        // unique_ptr will handle cleanup automatically
    }
};

// Test Case 1: Verify the first "ping" to "pong" transition.
TEST_F(ListHandler_LogicTest, ExecuteCurrentListAndCycle_WhenCalledOnce_SwitchesFillListTargetFrom1To2) {
    // Arrange
    ASSERT_EQ(listHandler->getCurrentFillListId(), 1) << "Initial fill list should be 1.";

    // Act
    listHandler->executeCurrentListAndCycle();

    // Assert
    EXPECT_EQ(listHandler->getCurrentFillListId(), 2);
}

// Test Case 2: Verify the "pong" back to "ping" transition.
TEST_F(ListHandler_LogicTest, ExecuteCurrentListAndCycle_WhenCalledTwice_SwitchesFillListTargetBackTo1) {
    // Arrange

    // Act
    listHandler->executeCurrentListAndCycle(); // State becomes 2
    listHandler->executeCurrentListAndCycle(); // State should become 1 again

    // Assert
    EXPECT_EQ(listHandler->getCurrentFillListId(), 1);
}

// Test Case 3: Verify a longer sequence.
TEST_F(ListHandler_LogicTest, ExecuteCurrentListAndCycle_WhenCalledThreeTimes_SwitchesFillListTargetTo2) {
    // Arrange

    // Act
    listHandler->executeCurrentListAndCycle(); // -> 2
    listHandler->executeCurrentListAndCycle(); // -> 1
    listHandler->executeCurrentListAndCycle(); // -> 2

    // Assert
    EXPECT_EQ(listHandler->getCurrentFillListId(), 2);
}