#include "MicroVectorProcessor.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>

using namespace RTC6;

// Static factory method
IMicroVectorProcessor::Ptr MicroVectorProcessor::Create(IListCommandBuilder* listBuilder, ILaserControl* laserControl) {
    return std::make_shared<MicroVectorProcessor>(listBuilder, laserControl);
}

MicroVectorProcessor::MicroVectorProcessor(IListCommandBuilder* listBuilder, ILaserControl* laserControl)
    : listBuilder_(listBuilder)
    , laserControl_(laserControl) {
    if (!listBuilder_ || !laserControl_) {
        throw std::invalid_argument("MicroVectorProcessor: Null pointer in constructor");
    }
}

void MicroVectorProcessor::ProcessMicroVectorPath(
    const std::vector<Point>& path,
    IListCommandBuilder* listBuilder,
    ILaserControl* laserControl
) {
    if (path.size() < 2) {
        throw std::invalid_argument("Path must contain at least 2 points");
    }

    IListCommandBuilder* builder = listBuilder ? listBuilder : listBuilder_;
    ILaserControl* laser = laserControl ? laserControl : laserControl_;

    if (!builder || !laser) {
        throw std::runtime_error("No valid command builder or laser control provided");
    }

    // Jump to start position
    builder->JumpAbsolute(path[0].X, path[0].Y);

    // Process each segment
    for (size_t i = 0; i < path.size() - 1; ++i) {
        const Point& start = path[i];
        const Point& end = path[i + 1];

        // Skip zero-length segments
        if (start.X == end.X && start.Y == end.Y && start.Z == end.Z) {
            continue;
        }

        processSegment(start, end, builder, laser);
    }
}

void MicroVectorProcessor::processSegment(
    const Point& start,
    const Point& end,
    IListCommandBuilder* builder,
    ILaserControl* laser
) const {
    const double distance = calculateDistance(start, end);
    
    // Skip very short segments
    if (distance < minSegmentLength_) {
        return;
    }

    const double speed = dynamicParamsEnabled_ 
        ? calculateSpeedForSegment(start, end, laser->GetSpeedForList())
        : laser->GetSpeedForList();

    const int numVectors = calculateNumberOfMicroVectors(distance, speed, timeStep_);

    if (dynamicParamsEnabled_) {
        // Set dynamic parameters for this segment
        builder->SetCurrentListLaserPower(laser->GetPowerForList());
        builder->SetCurrentListMarkSpeed(speed);
        builder->SetCurrentListZAxisHeight(end.Z);
    }

    // Turn laser on for this segment
    builder->AddLaserSignalOnList();

    // Generate micro-vectors for this segment
    for (int j = 0; j < numVectors; ++j) {
        const Point step = calculateStep(start, end, numVectors, j);
        const int dt = static_cast<int>(std::round(timeStep_ / RTC6::CLOCK_CYCLE));
        builder->AddMicroVector(step.X, step.Y, dt);
    }

    // Turn laser off at the end of the segment
    builder->AddLaserSignalOffList();
}

double MicroVectorProcessor::calculateSpeedForSegment(
    const Point& p1,
    const Point& p2,
    double defaultSpeed
) const {
    // Simple implementation - could be enhanced with more sophisticated logic
    // based on segment properties (length, angle, etc.)
    return defaultSpeed;
}

// ===== Configuration Methods =====

void MicroVectorProcessor::SetTimeStep(double timeStep) {
    if (timeStep < RTC6::MIN_TIME_TICK || timeStep > RTC6::MAX_TIME_TICK) {
        throw std::out_of_range("Time step out of valid range");
    }
    timeStep_ = timeStep;
}

double MicroVectorProcessor::GetTimeStep() const {
    return timeStep_;
}

void MicroVectorProcessor::SetMinimumSegmentLength(int minLength) {
    if (minLength < 0) {
        throw std::invalid_argument("Minimum segment length cannot be negative");
    }
    minSegmentLength_ = minLength;
}

int MicroVectorProcessor::GetMinimumSegmentLength() const {
    return minSegmentLength_;
}

void MicroVectorProcessor::EnableDynamicParameterAdjustment(bool enable) {
    dynamicParamsEnabled_ = enable;
}

bool MicroVectorProcessor::IsDynamicParameterAdjustmentEnabled() const {
    return dynamicParamsEnabled_;
}

// ===== Static Helper Methods =====

double MicroVectorProcessor::calculateDistance(const Point& p1, const Point& p2) {
    const double dx = static_cast<double>(p2.X - p1.X);
    const double dy = static_cast<double>(p2.Y - p1.Y);
    const double dz = static_cast<double>(p2.Z - p1.Z);
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Point MicroVectorProcessor::calculateStep(
    const Point& start,
    const Point& end,
    int totalSteps,
    int currentStep
) {
    if (totalSteps <= 0) {
        throw std::invalid_argument("Total steps must be positive");
    }
    if (currentStep < 0 || currentStep >= totalSteps) {
        throw std::out_of_range("Current step out of range");
    }

    const double t = static_cast<double>(currentStep + 1) / totalSteps;
    const int x = start.X + static_cast<int>(std::round((end.X - start.X) * t));
    const int y = start.Y + static_cast<int>(std::round((end.Y - start.Y) * t));
    
    static int lastX = start.X;
    static int lastY = start.Y;
    
    Point delta(x - lastX, y - lastY, 0);
    lastX = x;
    lastY = y;
    
    return delta;
}

int MicroVectorProcessor::calculateNumberOfMicroVectors(
    double distance,
    double speed,
    double timeStep
) {
    if (distance <= 0.0 || speed <= 0.0 || timeStep <= 0.0) {
        return 1;  // Minimum one vector even for zero/negative values
    }
    
    const double speedInBitsPerMicrosecond = speed / 1e6;  // Convert mm/s to bits/µs
    const double timeMicroseconds = distance / speedInBitsPerMicrosecond;
    const int numVectors = static_cast<int>(std::ceil(timeMicroseconds / timeStep));
    
    return std::max(1, numVectors);
}
