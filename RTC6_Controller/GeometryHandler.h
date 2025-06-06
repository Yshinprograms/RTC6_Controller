#pragma once

#include "ListHandler.h"
#include "Geometry.h"
#include <vector>

// Translates high-level geometric data into a sequence of low-level RTC6 list commands.
class GeometryHandler {
public:
    // The handler requires a ListHandler to send commands to.
    GeometryHandler(ListHandler& listHandler);

    // Processes a polyline with specified parameters.
    // Note: For this bare-minimum version, markSpeed is conceptual. Actual speed
    // depends on the density of points in the polyline and the fixed 10us
    // execution time of micro_vector_abs.
    void processPolyline(
        const std::vector<Point>& polyline,
        double laserPowerPercent,
        double markSpeed,
        double zPosition
    );

private:
    ListHandler& m_listHandler;

    // --- Unit Conversion Helpers ---
    // These are private as they are implementation details of this handler.

    // Converts real-world units (mm) to RTC6 internal units (bits).
    // This value MUST be calibrated for a specific optical setup.
    static constexpr double BITS_PER_MM = 1000.0;
    int mmToBits(double mm) const;

    // Converts a power percentage (0-100) to a 12-bit DAC value (0-4095).
    UINT powerToDAC(double percent) const;
};