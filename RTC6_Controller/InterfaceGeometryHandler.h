#pragma once
#include "open_vector_format.pb.h"

class InterfaceGeometryHandler {
public:
    virtual ~InterfaceGeometryHandler() = default;

    /**
     * @brief Processes a single geometric block and its associated parameters,
     *        translating them into low-level list commands.
     * @param block The OVF VectorBlock containing geometry like lines or hatches.
     * @param params The OVF MarkingParams containing settings like power and speed.
     */
    virtual void processVectorBlock(
        const open_vector_format::VectorBlock& block,
        const open_vector_format::MarkingParams& params) = 0;
};