// Geometry/MicroVectorProcessor.h
#ifndef MICRO_VECTOR_PROCESSOR_H
#define MICRO_VECTOR_PROCESSOR_H

#include "IMicroVectorProcessor.h"   // Inherit from interface
#include "IListCommandBuilder.h"    // Injected dependency
#include "ILaserControl.h"          // Injected dependency
#include "Rtc6Common.h"                        // For Point, UINT, LONG types
#include <vector>
#include <cmath>                                  // For std::sqrt, std::ceil, std::round
#include <memory>                                 // For std::unique_ptr
#include <stdexcept>                              // For std::invalid_argument

// Concrete implementation of IMicroVectorProcessor.
// This class is responsible for converting a high-level geometric path (series of points)
// into a sequence of low-level RTC6 micro-vector commands, including dynamic parameter control.
class MicroVectorProcessor : public IMicroVectorProcessor {
public:
    // Constructor: Takes non-owning pointers to required service interfaces.
    // These dependencies are injected from the higher-level orchestrator.
    explicit MicroVectorProcessor(IListCommandBuilder* listBuilder, ILaserControl* laserControl);

    // Destructor (defaulted as it owns no resources that need explicit cleanup).
    ~MicroVectorProcessor() override = default;

    // Prevent copying and assignment (good practice for resource-managing classes)
    MicroVectorProcessor(const MicroVectorProcessor&) = delete;
    MicroVectorProcessor& operator=(const MicroVectorProcessor&) = delete;
    MicroVectorProcessor(MicroVectorProcessor&&) = delete;
    MicroVectorProcessor& operator=(MicroVectorProcessor&&) = delete;

    // --- IMicroVectorProcessor Interface Implementations ---
    void ProcessMicroVectorPath(const std::vector<Point>& path) override;

    // Configuration Methods
    void SetMicroVectorTimeStep(double timeStep) override;
    double GetMicroVectorTimeStep() const override;
    void SetMinimumSegmentLength(LONG minLength) override;
    LONG GetMinimumSegmentLength() const override;
    void EnableDynamicParameterAdjustment(bool enable) override;
    bool IsDynamicParameterAdjustmentEnabled() const override;

private:
    // --- Injected Dependencies (non-owning pointers) ---
    IListCommandBuilder* listBuilder_;
    ILaserControl* laserControl_;

    // --- Internal Configuration Parameters ---
    double microVectorTimeStepUs_ = RTC6::CLOCK_CYCLE_MICROSECONDS; // Default 10µs
    LONG minSegmentLengthBits_ = 10;                              // Minimum segment length in bits (e.g., 10 bits)
    bool dynamicParamsEnabled_ = true;                            // Enable dynamic parameter adjustment within list

    // --- Private Helper Methods for Path Processing ---
    // Processes a single segment (straight line between two points) into micro-vectors.
    void processSegment(const Point& start, const Point& end) const;

    // --- Static Helper Methods (no access to class members) ---
    // Calculates Euclidean distance between two 3D points.
    static double calculateDistance(const Point& p1, const Point& p2);

    // Calculates the number of 10µs micro-vectors needed for a given distance and speed.
    // speed is in bits/ms, timeStep is in microseconds.
    static int calculateNumberOfMicroVectors(double distanceBits, double speedBitsPerMs, double microVectorTimeStepUs);

    // Calculates the absolute (X, Y) target point for a specific micro-vector step.
    // This function ensures the last micro-vector lands precisely on the end point.
    static Point calculateMicroVectorTarget(const Point& start, const Point& end, int totalMicroVectors, int currentVectorIndex);
};

#endif // MICRO_VECTOR_PROCESSOR_H