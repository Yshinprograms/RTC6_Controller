#pragma once
#include "gmock/gmock.h"
#include "InterfaceGeometryHandler.h"

class MockGeometryHandler : public InterfaceGeometryHandler {
public:
    MOCK_METHOD(void, processVectorBlock, (const open_vector_format::VectorBlock&, const open_vector_format::MarkingParams&), (override));
};