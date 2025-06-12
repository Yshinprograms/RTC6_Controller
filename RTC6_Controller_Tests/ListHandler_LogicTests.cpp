#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ListHandler.h"
#include "MockCommunicator.h"
#include "MockRtcApi.h"
#include <memory>

using ::testing::Return;
using ::testing::_;

class ListHandler_LogicTest : public ::testing::Test {
protected:
    std::unique_ptr<MockCommunicator> mockCommunicator;
    std::unique_ptr<MockRtcApi> mockRtcApi;
    std::unique_ptr<ListHandler> listHandler;

    void SetUp() override {
        mockCommunicator = std::make_unique<MockCommunicator>();
        mockRtcApi = std::make_unique<MockRtcApi>();

        ON_CALL(*mockCommunicator, isSuccessfullySetup()).WillByDefault(Return(true));

        ON_CALL(*mockRtcApi, api_execute_list(_)).WillByDefault(Return());

        listHandler = std::make_unique<ListHandler>(*mockCommunicator, *mockRtcApi);
    }

    void TearDown() override {
    }
};

TEST_F(ListHandler_LogicTest, ExecuteCurrentListAndCycle_WhenCalledOnce_SwitchesFillListTargetFrom1To2) {
    ASSERT_EQ(listHandler->getCurrentFillListId(), 1) << "Initial fill list should be 1.";

    listHandler->executeCurrentListAndCycle();

    EXPECT_EQ(listHandler->getCurrentFillListId(), 2);
}

TEST_F(ListHandler_LogicTest, ExecuteCurrentListAndCycle_WhenCalledTwice_SwitchesFillListTargetBackTo1) {
    listHandler->executeCurrentListAndCycle();
    listHandler->executeCurrentListAndCycle();

    EXPECT_EQ(listHandler->getCurrentFillListId(), 1);
}

TEST_F(ListHandler_LogicTest, ExecuteCurrentListAndCycle_WhenCalledThreeTimes_SwitchesFillListTargetTo2) {
    listHandler->executeCurrentListAndCycle();
    listHandler->executeCurrentListAndCycle();
    listHandler->executeCurrentListAndCycle();

    EXPECT_EQ(listHandler->getCurrentFillListId(), 2);
}