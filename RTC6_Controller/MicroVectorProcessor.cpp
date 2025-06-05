// Geometry/MicroVectorProcessor.cpp
#include "MicroVectorProcessor.h"
#include <algorithm> // For std::max, std::min
#include <iostream>  // For debugging output

// Constructor: Initializes with injected dependencies.
MicroVectorProcessor::MicroVectorProcessor(IListCommandBuilder* listBuilder, ILaserControl* laserControl)
    : listBuilder_(listBuilder)
    , laserControl_(laserControl) {
    if (!listBuilder_ || !laserControl_) {
        throw std::invalid_argument("MicroVectorProcessor: listBuilder or laserControl cannot be null.");
    }
    std::cout << "[MicroVectorProcessor] Instance created." << std::endl;
}

// --- IMicroVectorProcessor Interface Implementations ---

// Processes a path of points into a sequence of micro-vector commands.
void MicroVectorProcessor::ProcessMicroVectorPath(const std::vector<Point>& path) {
    if (path.size() < 2) {
        throw std::invalid_argument("MicroVectorProcessor: Path must contain at least 2 points.");
    }
    std::cout << "[MicroVectorProcessor] Processing path with " << path.size() << " points." << std::endl;

    // Jump to the start of the first segment (laser off).
    // Note: This jump is to the start of the *first segment*, not necessarily the start of the *first micro-vector*.
    listBuilder_->JumpAbsolute(path[0].X, path[0].Y);

    // Process each segment of the path.
    for (size_t i = 0; i < path.size() - 1; ++i) {
        const Point& start = path[i];
        const Point& end = path[i + 1];

        // Skip zero-length segments to avoid division by zero or unnecessary processing.
        if (start.X == end.X && start.Y == end.Y && start.Z == end.Z) {
            continue;
        }

        processSegment(start, end); // Process individual segment
    }
    std::cout << "[MicroVectorProcessor] Path processing complete." << std::endl;
}

// Configuration Methods
void MicroVectorProcessor::SetMicroVectorTimeStep(double timeStep) {
    // Basic validation, RTC6 clock cycle is 10us.
    if (timeStep < RTC6::CLOCK_CYCLE_MICROSECONDS || timeStep > 1000.0) { // e.g., max 1ms
        throw std::out_of_range("MicroVectorProcessor: Time step must be between 10us and 1ms.");
    }
    microVectorTimeStepUs_ = timeStep;
    std::cout << "[MicroVectorProcessor] Micro-vector time step set to: " << microVectorTimeStepUs_ << "us." << std::endl;
}

double MicroVectorProcessor::GetMicroVectorTimeStep() const {
    return microVectorTimeStepUs_;
}

void MicroVectorProcessor::SetMinimumSegmentLength(LONG minLength) {
    if (minLength < 0) {
        throw std::invalid_argument("MicroVectorProcessor: Minimum segment length cannot be negative.");
    }
    minSegmentLengthBits_ = minLength;
    std::cout << "[MicroVectorProcessor] Minimum segment length set to: " << minSegmentLengthBits_ << " bits." << std::endl;
}

LONG MicroVectorProcessor::GetMinimumSegmentLength() const {
    return minSegmentLengthBits_;
}

void MicroVectorProcessor::EnableDynamicParameterAdjustment(bool enable) {
    dynamicParamsEnabled_ = enable;
    std::cout << "[MicroVectorProcessor] Dynamic parameter adjustment " << (enable ? "enabled" : "disabled") << "." << std::endl;
}

bool MicroVectorProcessor::IsDynamicParameterAdjustmentEnabled() const {
    return dynamicParamsEnabled_;
}

// --- Private Helper Methods for Path Processing ---

// Processes a single segment (straight line between two points) into micro-vectors.
void MicroVectorProcessor::processSegment(const Point& start, const Point& end) const {
    const double distanceBits = calculateDistance(start, end);

    // Skip very short segments based on configuration.
    if (distanceBits < minSegmentLengthBits_) {
        std::cout << "[MicroVectorProcessor] Skipping very short segment (distance: " << distanceBits << " bits)." << std::endl;
        return;
    }

    // Get current speed from LaserControl for this segment.
    const double speedBitsPerMs = laserControl_->GetSpeedForList();

    // Calculate number of micro-vectors needed for this segment.
    const int numMicroVectors = calculateNumberOfMicroVectors(distanceBits, speedBitsPerMs, microVectorTimeStepUs_);

    // Set dynamic parameters for this segment if enabled.
    if (dynamicParamsEnabled_) {
        listBuilder_->SetCurrentListLaserPower(laserControl_->GetPowerForList());
        listBuilder_->SetCurrentListMarkSpeed(speedBitsPerMs);
        listBuilder_->SetCurrentListZAxisHeight(laserControl_->GetZAxisHeightForList());
    }

    // Generate and add each individual micro-vector.
    // Micro-vectors are always 10us steps in RTC6.
    // The laser state is controlled directly by the micro_vector_abs command.
    for (int j = 0; j < numMicroVectors; ++j) {
        // Calculate the absolute target point for this micro-vector step.
        const Point targetPoint = calculateMicroVectorTarget(start, end, numMicroVectors, j);

        // Add the micro-vector command to the list.
        // Laser is ON for all micro-vectors within a marking segment.
        listBuilder_->AddMicroVector(
            targetPoint.X,
            targetPoint.Y,
            laserControl_->GetLaserOnBitValue(), // Laser ON state bits
            laserControl_->GetLaserOffBitValue() // Laser OFF state bits (for 10us cycle)
        );
    }
    std::cout << "[MicroVectorProcessor] Processed segment from (" << start.X << "," << start.Y << "," << start.Z << ") to ("
        << end.X << "," << end.Y << "," << end.Z << ") with " << numMicroVectors << " micro-vectors." << std::endl;
}

// --- Static Helper Methods ---

// Calculates Euclidean distance between two 3D points.
double MicroVectorProcessor::calculateDistance(const Point& p1, const Point& p2) {
    const double dx = static_cast<double>(p2.X - p1.X);
    const double dy = static_cast<double>(p2.Y - p1.Y);
    const double dz = static_cast<double>(p2.Z - p1.Z); // Include Z for distance calculation
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// Calculates the number of micro-vectors needed for a given distance and speed.
// Speed is in bits/ms, timeStep is in microseconds.
int MicroVectorProcessor::calculateNumberOfMicroVectors(
    double distanceBits,
    double speedBitsPerMs,
    double microVectorTimeStepUs
) {
    if (distanceBits < 0.0 || speedBitsPerMs <= 0.0 || microVectorTimeStepUs <= 0.0) {
        return 1; // Always at least one micro-vector for a segment
    }

    // Convert speed to bits per microsecond.
    const double speedBitsPerUs = speedBitsPerMs / 1000.0; // 1 ms = 1000 us

    // Calculate total time in microseconds required for the distance at the given speed.
    const double totalTimeUs = distanceBits / speedBitsPerUs;

    // Calculate number of micro-vectors (each is microVectorTimeStepUs long).
    // Use ceil to ensure enough vectors are generated to cover the full distance.
    int numVectors = static_cast<int>(std::ceil(totalTimeUs / microVectorTimeStepUs));

    // Ensure at least one micro-vector is generated for any valid segment.
    return std::max(1, numVectors);
}

// Calculates the absolute (X, Y) target point for a specific micro-vector step.
// This function ensures the last micro-vector lands precisely on the end point.
Point MicroVectorProcessor::calculateMicroVectorTarget(
    const Point& start,
    const Point& end,
    int totalMicroVectors,
    int currentVectorIndex
) {
    if (totalMicroVectors <= 0) {
        throw std::invalid_argument("calculateMicroVectorTarget: totalMicroVectors must be positive.");
    }
    if (currentVectorIndex < 0 || currentVectorIndex >= totalMicroVectors) {
        throw std::out_of_range("calculateMicroVectorTarget: currentVectorIndex out of range.");
    }

    // Interpolation factor (t) from 0.0 to 1.0, where 1.0 is reached at the *last* vector's target.
    const double t = static_cast<double>(currentVectorIndex + 1) / totalMicroVectors;

    // Calculate the interpolated X and Y coordinates.
    // Use std::round for accurate integer conversion.
    const LONG x = start.X + static_cast<LONG>(std::round((end.X - start.X) * t));
    const LONG y = start.Y + static_cast<LONG>(std::round((end.Y - start.Y) * t));
    const LONG z = start.Z + static_cast<LONG>(std::round((end.Z - start.Z) * t));

    // Note: micro_vector_abs does not directly take Z. Z changes are via set_defocus_list.
    // The Z here is only for accurate distance calculation and might be used for future 3D micro_vector commands.
    return Point(x, y, z);
}