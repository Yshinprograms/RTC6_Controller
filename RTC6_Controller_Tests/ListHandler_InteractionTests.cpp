#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ListHandler.h"
#include "MockCommunicator.h"
#include "MockRtcApi.h"
#include "Rtc6Constants.h"
#include <memory>

using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DoubleEq;

class ListHandler_InteractionTest : public ::testing::Test {
protected:
    std::unique_ptr<NiceMock<MockCommunicator>> mockCommunicator;
    std::unique_ptr<NiceMock<MockRtcApi>> mockRtcApi;
    std::unique_ptr<ListHandler> listHandler;

    void SetUp() override {
        mockCommunicator = std::make_unique<NiceMock<MockCommunicator>>();
        mockRtcApi = std::make_unique<NiceMock<MockRtcApi>>();

        ON_CALL(*mockCommunicator, isSuccessfullySetup()).WillByDefault(Return(true));

        listHandler = std::make_unique<ListHandler>(*mockCommunicator, *mockRtcApi);
    }

    void TearDown() override {
    }
};


TEST_F(ListHandler_InteractionTest, SetupAutoChangeMode_WhenCommunicatorIsReady_CallsApiAutoChange) {
    EXPECT_CALL(*mockRtcApi, api_auto_change()).Times(1);

    listHandler->setupAutoChangeMode();
}

TEST_F(ListHandler_InteractionTest, SetupAutoChangeMode_WhenCommunicatorNotReady_DoesNotCallApiAutoChange) {
    EXPECT_CALL(*mockCommunicator, isSuccessfullySetup()).WillOnce(Return(false));

    EXPECT_CALL(*mockRtcApi, api_auto_change()).Times(0);

    listHandler->setupAutoChangeMode();
}


TEST_F(ListHandler_InteractionTest, BeginListPreparation_OnFirstCall_CallsSetStartListWithList1) {
    EXPECT_CALL(*mockRtcApi, api_set_start_list(1)).Times(1);

    listHandler->beginListPreparation();
}

TEST_F(ListHandler_InteractionTest, BeginListPreparation_AfterOneCycle_CallsSetStartListWithList2) {
    // Arrange: Change the internal state of the ListHandler first.
    listHandler->executeCurrentListAndCycle();

    EXPECT_CALL(*mockRtcApi, api_set_start_list(2)).Times(1);

    listHandler->beginListPreparation();
}


TEST_F(ListHandler_InteractionTest, ExecuteCurrentListAndCycle_OnFirstCall_CallsExecuteListWithList1) {
    EXPECT_CALL(*mockRtcApi, api_execute_list(1)).Times(1);

    listHandler->executeCurrentListAndCycle();
}

TEST_F(ListHandler_InteractionTest, ExecuteCurrentListAndCycle_AfterOneCycle_CallsExecuteListWithList2) {
    // Arrange: Perform one full cycle to change the internal state.
    listHandler->executeCurrentListAndCycle();

    EXPECT_CALL(*mockRtcApi, api_execute_list(2)).Times(1);

    listHandler->executeCurrentListAndCycle();
}


TEST_F(ListHandler_InteractionTest, IsListBusy_WhenApiReportsBusy1_ReturnsTrueForList1) {
    EXPECT_CALL(*mockRtcApi, api_read_status()).WillOnce(Return(Rtc6Constants::Status::BUSY1));

    EXPECT_TRUE(listHandler->isListBusy(1));
}

TEST_F(ListHandler_InteractionTest, IsListBusy_WhenApiReportsBusy2_ReturnsTrueForList2) {
    EXPECT_CALL(*mockRtcApi, api_read_status()).WillOnce(Return(Rtc6Constants::Status::BUSY2));

    EXPECT_TRUE(listHandler->isListBusy(2));
}

TEST_F(ListHandler_InteractionTest, IsListBusyForList1_WhenApiReportsNotBusy_ReturnsFalse) {
    EXPECT_CALL(*mockRtcApi, api_read_status()).WillOnce(Return(0));

    EXPECT_FALSE(listHandler->isListBusy(1));
}

TEST_F(ListHandler_InteractionTest, IsListBusyForList2_WhenApiReportsNotBusy_ReturnsFalse) {
    EXPECT_CALL(*mockRtcApi, api_read_status()).WillOnce(Return(0));

    EXPECT_FALSE(listHandler->isListBusy(2));
}

TEST_F(ListHandler_InteractionTest, IsListBusy_WhenCommunicatorNotReady_ReturnsTrueAndDoesNotCallApi) {
    EXPECT_CALL(*mockCommunicator, isSuccessfullySetup()).WillRepeatedly(Return(false));

    EXPECT_CALL(*mockRtcApi, api_read_status()).Times(0);

    // The method should return true as a safety measure.
    EXPECT_TRUE(listHandler->isListBusy(1));
}


TEST_F(ListHandler_InteractionTest, AddJumpAbsolute_WithCoordinates_CallsApiJumpAbsWithSameCoordinates) {
    const INT testX = 12345;
    const INT testY = -54321;
    EXPECT_CALL(*mockRtcApi, api_jump_abs(testX, testY)).Times(1);

    listHandler->addJumpAbsolute(testX, testY);
}

TEST_F(ListHandler_InteractionTest, AddMarkAbsolute_WithCoordinates_CallsApiMarkAbsWithSameCoordinates) {
    const INT testX = 9876;
    const INT testY = -1234;
    EXPECT_CALL(*mockRtcApi, api_mark_abs(testX, testY)).Times(1);

    listHandler->addMarkAbsolute(testX, testY);
}

TEST_F(ListHandler_InteractionTest, AddSetFocusOffset_WithOffset_CallsApiSetDefocusWithSameOffset) {
    const INT offset = -2048;
    EXPECT_CALL(*mockRtcApi, api_set_defocus_list(offset)).Times(1);

    listHandler->addSetFocusOffset(offset);
}

TEST_F(ListHandler_InteractionTest, AddSetLaserPower_WithPortAndPower_CallsApiSetLaserPowerWithSameValues) {
    const UINT port = 1;
    const UINT power = 3500;
    EXPECT_CALL(*mockRtcApi, api_set_laser_power(port, power)).Times(1);

    listHandler->addSetLaserPower(port, power);
}

TEST_F(ListHandler_InteractionTest, AddSetMarkSpeed_WithSpeedInMmPerS_CallsApiWithCorrectlyCalculatedBitsPerMs) {
    const double speed_mm_s = 1000.0;
    const double expected_bits_per_ms = 1000.0; // (1000 * 1000) / 1000 = 1000 bits/ms

    // We use a floating-point matcher to avoid precision issues.
    EXPECT_CALL(*mockRtcApi, api_set_mark_speed(DoubleEq(expected_bits_per_ms))).Times(1);

    listHandler->addSetMarkSpeed(speed_mm_s);
}