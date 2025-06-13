#pragma once

#include "InterfaceListHandler.h"
#include "ProcessData.h"
#include <vector>

class GeometryHandler_PrivateMethodsTest;
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
    GeometryHandler(InterfaceListHandler& listHandler);
    ~GeometryHandler();

    // Processes a vector of points as a continuous polyline.
    // This method sets the necessary process parameters (speed, power, focus)
    // and then generates the appropriate jump and mark commands.
    void processPolyline(
        const std::vector<Point>& polyline, // The geometry to draw
        double laserPowerPercent,           // Laser power [0-100]
        double markSpeed_mm_s,              // Desired marking speed in mm/s
        double focusOffset_mm               // Optical Z-offset in mm
    );

private:
    friend class GeometryHandler_LogicTest;

    InterfaceListHandler& m_listHandler;

    // Defines how many "bits" the RTC6 needs to move the mirrors to cover 1 mm in the image field.
    // This value MUST be determined experimentally for a specific optical setup.
    static constexpr double BITS_PER_MM = 1000.0;

    int mmToBits(double mm) const;

    // Converts a laser power percentage (0-100) to a 12-bit DAC value (0-4095).
    UINT powerToDAC(double percent) const;
};