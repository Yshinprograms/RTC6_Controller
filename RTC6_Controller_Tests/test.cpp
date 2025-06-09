#include "pch.h"

class IExample {
public:
    virtual int GetValue() = 0;
};

class MockExample : public IExample {
public:
    MOCK_METHOD(int, GetValue, (), (override));
};

TEST(MockVerification, BasicMock) {
    MockExample mock;
    EXPECT_CALL(mock, GetValue())
        .WillOnce(testing::Return(42));

    ASSERT_EQ(mock.GetValue(), 42);
}

TEST(CrashCheck, InvalidAccess) {
    int* ptr = nullptr;
    *ptr = 42; // Force crash  
}



