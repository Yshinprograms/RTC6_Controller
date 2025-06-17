#pragma once

#include "InterfaceListHandler.h"
#include "open_vector_format.pb.h"
#include <vector>

// Forward declaration for the test class
class GeometryHandler_LogicTest;

class GeometryHandler {
public:
    GeometryHandler(InterfaceListHandler& listHandler);
    ~GeometryHandler();

    // The new, more generic processing method.
    // It takes the official Protobuf objects as direct input.
    void processVectorBlock(
        const open_vector_format::VectorBlock& block,
        const open_vector_format::MarkingParams& params
    );

private:
    // This allows your unit test to access the private helper methods.
    friend class GeometryHandler_LogicTest;

    InterfaceListHandler& m_listHandler;
    static constexpr double BITS_PER_MM = 1000.0;
    static constexpr double MAX_LASER_POWER_W = 100.0; // Define max power for conversion

    // These helpers remain unchanged but are now private
    int mmToBits(double mm) const;
    UINT powerToDAC(double percent) const;
};