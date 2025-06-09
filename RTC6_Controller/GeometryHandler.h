#pragma once

#include "InterfaceListHandler.h"
#include "Geometry.h"
#include <vector>

// -----------------------------------------------------------------------------
// GeometryHandler Class
// -----------------------------------------------------------------------------
// Purpose:
// Acts as a translator between high-level geometric concepts (like polylines
// with real-world units) and the low-level list commands required by the
// RTC6 board. It encapsulates the logic for converting units and structuring
// command sequences for common marking tasks.
// -----------------------------------------------------------------------------
class GeometryHandler {
public:
    // Constructor: Requires a reference to a ListHandler to send commands to.
    // This dependency is injected, making the class more modular and testable.
    GeometryHandler(InterfaceListHandler& listHandler);

    // Processes a vector of points as a continuous polyline.
    // This method sets the necessary process parameters (speed, power, focus)
    // and then generates the appropriate jump and mark commands.
    void processPolyline(
        const std::vector<Point>& polyline, // The geometry to draw
        double laserPowerPercent,           // Laser power [0-100]
        double markSpeed_mm_s,              // Desired marking speed in mm/s
        double focusOffset_mm               // Optical Z-offset in mm
    );

    // Future methods could be added here for other shapes:
    // void processCircle(Point center, double radius, ...);
    // void processHatchFill(const std::vector<Point>& boundary, ...);

private:
    InterfaceListHandler& m_listHandler; // A reference to the object that builds the command list.

    // --- Unit Conversion Helpers ---
    // These are kept private as they are implementation details.

    // This calibration constant is critical. It defines how many "bits"
    // the RTC6 needs to move the mirrors to cover 1 mm in the image field.
    // This value MUST be determined experimentally for a specific optical setup.
    static constexpr double BITS_PER_MM = 1000.0;

    // Converts real-world units (mm) to RTC6 internal units (bits).
    int mmToBits(double mm) const;

    // Converts a laser power percentage (0-100) to a 12-bit DAC value (0-4095).
    UINT powerToDAC(double percent) const;
};