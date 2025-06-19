#pragma once

#include <gmock/gmock.h>
#include <InterfaceUI.h>

class MockUI : public InterfaceUI {
public:
    MOCK_METHOD(void, displayMessage, (const std::string& message), (override));
    MOCK_METHOD(void, displayError, (const std::string& message), (override));
    MOCK_METHOD(void, displayProgress, (const std::string& message, int current, int total), (override));
};