#include "GeometryHandler.h"
#include <iostream>
#include <cmath> // For std::round

// Constructor implementation. Simply initializes the member reference.
GeometryHandler::GeometryHandler(ListHandler& listHandler)
    : m_listHandler(listHandler) {
    std::cout << "[GeometryHandler] Instance created." << std::endl;
}

// The core logic for processing a polyline using mark_abs for speed control.
void GeometryHandler::processPolyline(
    const std::vector<Point>& polyline,
    double laserPowerPercent,
    double markSpeed_mm_s,
    double focusOffset_mm) {

    // Guard clause: A polyline needs at least a start and an end point.
    if (polyline.size() < 2) {
        std::cerr << "WARNING: [GeometryHandler] Cannot process polyline with fewer than 2 points." << std::endl;
        return;
    }

    std::cout << "[GeometryHandler] Processing polyline with " << polyline.size()
        << " points using mark_abs at " << markSpeed_mm_s << " mm/s." << std::endl;

    // --- Step 1: Set parameters that apply to the entire polyline ---
    // These commands are modal; they affect all subsequent relevant commands
    // until they are changed again. They must be set before the geometry.
    m_listHandler.addSetMarkSpeed(markSpeed_mm_s);
    m_listHandler.addSetLaserPower(1, powerToDAC(laserPowerPercent)); // Port 1 is typical for laser power.
    m_listHandler.addSetFocusOffset(mmToBits(focusOffset_mm));

    // --- Step 2: Jump to the starting point of the polyline (laser off) ---
    const auto& startPoint = polyline.front();
    m_listHandler.addJumpAbsolute(mmToBits(startPoint.x), mmToBits(startPoint.y));

    // --- Step 3: Trace all segments of the polyline (laser on) ---
    // The RTC6 automatically handles turning the laser on for the first mark_abs
    // and keeping it on for the subsequent, consecutive mark_abs calls. It will
    // insert the correct 'Polygon Delay' at each corner.
    for (size_t i = 1; i < polyline.size(); ++i) {
        const auto& p = polyline[i];
        m_listHandler.addMarkAbsolute(mmToBits(p.x), mmToBits(p.y));
    }

    // The laser is automatically turned off by the RTC6 after this sequence,
    // because the next command in the list will either be another jump
    // or the 'set_end_of_list' command.
}

// Converts millimeters to RTC6 bits.
int GeometryHandler::mmToBits(double mm) const {
    return static_cast<int>(std::round(mm * BITS_PER_MM));
}

// Converts a 0-100% power value to a 0-4095 DAC value.
UINT GeometryHandler::powerToDAC(double percent) const {
    if (percent < 0.0) percent = 0.0;
    if (percent > 100.0) percent = 100.0;
    return static_cast<UINT>((percent / 100.0) * 4095.0);
}