// Ovf_Core/ProcessData.h

#pragma once

#include <vector>

// --- START of content moved from Geometry.h ---
struct Point {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};
// --- END of content moved from Geometry.h ---

// A clean, simple struct to hold the processing parameters we care about,
// translated from the OVF format into the units our GeometryHandler expects.
struct OvfProcessParameters {
    double laserPowerPercent = 50.0;
    double markSpeed_mm_s = 1000.0;
    double focusOffset_mm = 0.0;
};

// Represents a single, continuous polyline with its associated parameters.
// This is the fundamental "package" of work our GeometryHandler will process.
struct OvfPolyline {
    OvfProcessParameters params;
    std::vector<Point> points;
};

// Represents all the geometry within a single Z-height (a single layer).
struct OvfLayer {
    float z_height_mm = 0.0;
    std::vector<OvfPolyline> polylines;
};