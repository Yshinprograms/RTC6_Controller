#include "GeometryHandler.h"
#include <iostream>
#include <cmath> // For std::round

GeometryHandler::GeometryHandler(ListHandler& listHandler)
    : m_listHandler(listHandler) {
    std::cout << "[GeometryHandler] Instance created." << std::endl;
}

void GeometryHandler::processPolyline(
    const std::vector<Point>& polyline,
    double laserPowerPercent,
    double markSpeed,
    double zPosition) {

    if (polyline.size() < 2) {
        std::cerr << "WARNING: [GeometryHandler] Cannot process polyline with fewer than 2 points." << std::endl;
        return;
    }

    std::cout << "[GeometryHandler] Processing polyline with " << polyline.size() << " points." << std::endl;

    // 1. Set parameters for the entire polyline (Power & z-offset).
    m_listHandler.addSetLaserPower(1, powerToDAC(laserPowerPercent)); // Port 1 for ANALOG_OUT1 is common for laser power.
    m_listHandler.addSetZPosition(mmToBits(zPosition));

    // 2. Jump to the starting point with the laser off.
    const auto& startPoint = polyline.front();
    m_listHandler.addJumpAbsolute(mmToBits(startPoint.x), mmToBits(startPoint.y));

    // 3. Turn the laser on.
    m_listHandler.addLaserSignalOn();

    // 4. Trace the segments using micro_vector_abs.
    // Start from the second point since we already jumped to the first.
    for (size_t i = 1; i < polyline.size(); ++i) {
        const auto& p = polyline[i];
        // Use -1 for delays to indicate "use previously set value", a safe default.
        m_listHandler.addMicroVectorAbs(mmToBits(p.x), mmToBits(p.y), -1, -1);
    }

    // 5. Turn the laser off after the last vector.
    m_listHandler.addLaserSignalOff();
}

int GeometryHandler::mmToBits(double mm) const {
    return static_cast<int>(std::round(mm * BITS_PER_MM));
}

UINT GeometryHandler::powerToDAC(double percent) const {
    if (percent < 0.0) percent = 0.0;
    if (percent > 100.0) percent = 100.0;
    // Map 0-100% to a 12-bit DAC value (0-4095)
    return static_cast<UINT>((percent / 100.0) * 4095.0);
}