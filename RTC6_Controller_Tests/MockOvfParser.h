#pragma once
#include "gmock/gmock.h"
#include "InterfaceOvfParser.h"

class MockOvfParser : public InterfaceOvfParser {
public:
    MOCK_METHOD(bool, openFile, (const std::string& filePath), (override));
    MOCK_METHOD(int, getNumberOfWorkPlanes, (), (const, override));
    MOCK_METHOD(open_vector_format::Job, getJobShell, (), (const, override));
    MOCK_METHOD(open_vector_format::WorkPlane, getWorkPlane, (int index), (override));
};
