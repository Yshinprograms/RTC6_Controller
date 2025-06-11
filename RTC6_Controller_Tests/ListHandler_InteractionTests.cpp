#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ListHandler.h"
#include "MockCommunicator.h"
#include "MockRtcApi.h"
#include "Rtc6Constants.h" // Needed for status bits
#include <memory>

// Use testing namespace for brevity in tests
using ::testing::Return;
using ::testing::_; // A wildcard matcher for any argument
using ::testing::NiceMock;
using ::testing::DoubleEq;

// This test suite verifies the interactions between ListHandler and its dependencies.
// The "Golden Rule" is applied: ListHandler is the REAL object under test,
// while its dependencies (InterfaceCommunicator, InterfaceRtcApi) are MOCKED.
// This allows us to "wiretap" the calls and verify ListHandler behaves correctly.
class ListHandler_InteractionTest : public ::testing::Test {
protected:
    // We use NiceMock to suppress warnings about "uninteresting" mock calls.
    // This is useful when a mock is called in ways we don't care about for a specific test.
    std::unique_ptr<NiceMock<MockCommunicator>> mockCommunicator;
    std::unique_ptr<NiceMock<MockRtcApi>> mockRtcApi;

    // The REAL object under test
    std::unique_ptr<ListHandler> listHandler;

    void SetUp() override {
        // 1. Create the mock dependencies
        mockCommunicator = std::make_unique<NiceMock<MockCommunicator>>();
        mockRtcApi = std::make_unique<NiceMock<MockRtcApi>>();

        // 2. Set up default "happy path" behavior.
        // Most methods require the communicator to be ready, so we set this as the default.
        // Individual tests can override this behavior if they need to test the "not ready" state.
        ON_CALL(*mockCommunicator, isSuccessfullySetup()).WillByDefault(Return(true));

        // 3. Create the REAL object under test, injecting the mocks.
        listHandler = std::make_unique<ListHandler>(*mockCommunicator, *mockRtcApi);
    }

    void TearDown() override {
        // unique_ptr handles all memory management automatically.
    }
};


// --- Tests for Method: setupAutoChangeMode ---

TEST_F(ListHandler_InteractionTest, SetupAutoChangeMode_WhenCommunicatorIsReady_CallsApiAutoChange) {
    // Arrange: Set expectation on the mock API
    EXPECT_CALL(*mockRtcApi, api_auto_change()).Times(1);

    // Act: Call the method under test
    listHandler->setupAutoChangeMode();
}

TEST_F(ListHandler_InteractionTest, SetupAutoChangeMode_WhenCommunicatorNotReady_DoesNotCallApiAutoChange) {
    // Arrange: Override the default setup for this specific test case.
    EXPECT_CALL(*mockCommunicator, isSuccessfullySetup()).WillOnce(Return(false));

    // Arrange: Expect the API to NOT be called.
    EXPECT_CALL(*mockRtcApi, api_auto_change()).Times(0);

    // Act
    listHandler->setupAutoChangeMode();
}


// --- Tests for Method: beginListPreparation ---

TEST_F(ListHandler_InteractionTest, BeginListPreparation_OnFirstCall_CallsSetStartListWithList1) {
    // Arrange
    EXPECT_CALL(*mockRtcApi, api_set_start_list(1)).Times(1);

    // Act
    listHandler->beginListPreparation();
}

TEST_F(ListHandler_InteractionTest, BeginListPreparation_AfterOneCycle_CallsSetStartListWithList2) {
    // Arrange: Change the internal state of the ListHandler first.
    listHandler->executeCurrentListAndCycle();

    // Arrange: Now, set the expectation for the next call.
    EXPECT_CALL(*mockRtcApi, api_set_start_list(2)).Times(1);

    // Act
    listHandler->beginListPreparation();
}


// --- Tests for Method: executeCurrentListAndCycle ---

TEST_F(ListHandler_InteractionTest, ExecuteCurrentListAndCycle_OnFirstCall_CallsExecuteListWithList1) {
    // Arrange
    EXPECT_CALL(*mockRtcApi, api_execute_list(1)).Times(1);

    // Act
    listHandler->executeCurrentListAndCycle();
}

TEST_F(ListHandler_InteractionTest, ExecuteCurrentListAndCycle_AfterOneCycle_CallsExecuteListWithList2) {
    // Arrange: Perform one full cycle to change the internal state.
    listHandler->executeCurrentListAndCycle(); // Executes list 1, next to fill is 2

    // Arrange: Now expect the next execution to be on list 2.
    EXPECT_CALL(*mockRtcApi, api_execute_list(2)).Times(1);

    // Act
    listHandler->executeCurrentListAndCycle();
}


// --- Tests for Method: isListBusy ---

TEST_F(ListHandler_InteractionTest, IsListBusy_WhenApiReportsBusy1_ReturnsTrueForList1) {
    // Arrange: Stub the return value of the API call.
    // We tell the mock to return a status word where the BUSY1 bit is set.
    EXPECT_CALL(*mockRtcApi, api_read_status()).WillOnce(Return(Rtc6Constants::Status::BUSY1));

    // Act & Assert
    EXPECT_TRUE(listHandler->isListBusy(1));
}

TEST_F(ListHandler_InteractionTest, IsListBusy_WhenApiReportsBusy2_ReturnsTrueForList2) {
    // Arrange
    EXPECT_CALL(*mockRtcApi, api_read_status()).WillOnce(Return(Rtc6Constants::Status::BUSY2));

    // Act & Assert
    EXPECT_TRUE(listHandler->isListBusy(2));
}

TEST_F(ListHandler_InteractionTest, IsListBusyForList1_WhenApiReportsNotBusy_ReturnsFalse) {
    // Arrange: Expect one call, which will be consumed by the single Act line.
    EXPECT_CALL(*mockRtcApi, api_read_status()).WillOnce(Return(0)); // No bits set

    // Act & Assert
    EXPECT_FALSE(listHandler->isListBusy(1));
}

TEST_F(ListHandler_InteractionTest, IsListBusyForList2_WhenApiReportsNotBusy_ReturnsFalse) {
    // Arrange: Expect one call for this test's context.
    EXPECT_CALL(*mockRtcApi, api_read_status()).WillOnce(Return(0)); // No bits set

    // Act & Assert
    EXPECT_FALSE(listHandler->isListBusy(2));
}

TEST_F(ListHandler_InteractionTest, IsListBusy_WhenCommunicatorNotReady_ReturnsTrueAndDoesNotCallApi) {
    // Arrange: Override default setup.
    EXPECT_CALL(*mockCommunicator, isSuccessfullySetup()).WillRepeatedly(Return(false));

    // Arrange: Expect the API *not* to be called, as a failsafe.
    EXPECT_CALL(*mockRtcApi, api_read_status()).Times(0);

    // Act & Assert: The method should return true as a safety measure.
    EXPECT_TRUE(listHandler->isListBusy(1));
}


// --- Tests for "add" Commands (Pass-Through Verification) ---

TEST_F(ListHandler_InteractionTest, AddJumpAbsolute_WithCoordinates_CallsApiJumpAbsWithSameCoordinates) {
    // Arrange
    const INT testX = 12345;
    const INT testY = -54321;
    EXPECT_CALL(*mockRtcApi, api_jump_abs(testX, testY)).Times(1);

    // Act
    listHandler->addJumpAbsolute(testX, testY);
}

TEST_F(ListHandler_InteractionTest, AddMarkAbsolute_WithCoordinates_CallsApiMarkAbsWithSameCoordinates) {
    // Arrange
    const INT testX = 9876;
    const INT testY = -1234;
    EXPECT_CALL(*mockRtcApi, api_mark_abs(testX, testY)).Times(1);

    // Act
    listHandler->addMarkAbsolute(testX, testY);
}

TEST_F(ListHandler_InteractionTest, AddSetFocusOffset_WithOffset_CallsApiSetDefocusWithSameOffset) {
    // Arrange
    const INT offset = -2048;
    EXPECT_CALL(*mockRtcApi, api_set_defocus_list(offset)).Times(1);

    // Act
    listHandler->addSetFocusOffset(offset);
}

TEST_F(ListHandler_InteractionTest, AddSetLaserPower_WithPortAndPower_CallsApiSetLaserPowerWithSameValues) {
    // Arrange
    const UINT port = 1;
    const UINT power = 3500;
    EXPECT_CALL(*mockRtcApi, api_set_laser_power(port, power)).Times(1);

    // Act
    listHandler->addSetLaserPower(port, power);
}

// This test is particularly important as it verifies a calculation.
TEST_F(ListHandler_InteractionTest, AddSetMarkSpeed_WithSpeedInMmPerS_CallsApiWithCorrectlyCalculatedBitsPerMs) {
    // Arrange
    const double speed_mm_s = 1000.0; // 1000 mm/s
    const double expected_bits_per_ms = 1000.0; // (1000 * 1000) / 1000 = 1000 bits/ms

    // We use a floating-point matcher to avoid precision issues.
    EXPECT_CALL(*mockRtcApi, api_set_mark_speed(DoubleEq(expected_bits_per_ms))).Times(1);

    // Act
    listHandler->addSetMarkSpeed(speed_mm_s);
}