#include <gmock/gmock.h>
#include <gtest/gtest.h>

class Turtle {
public:
    virtual ~Turtle() {}
    virtual void PenUp() = 0;
    virtual void PenDown() = 0;
    virtual void Forward(int distance) = 0;
};

class MockTurtle : public Turtle {
public:
    MOCK_METHOD(void, PenUp, (), (override));
    MOCK_METHOD(void, PenDown, (), (override));
    MOCK_METHOD(void, Forward, (int distance), (override));
};

TEST(TurtleTest, PenUpTest) {
    MockTurtle turtle;
    EXPECT_CALL(turtle, PenUp()).Times(1);
    turtle.PenUp();
}
