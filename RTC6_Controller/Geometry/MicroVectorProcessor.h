#ifndef MICRO_VECTOR_PROCESSOR_H
#define MICRO_VECTOR_PROCESSOR_H

#include "../Interfaces/IMicroVectorProcessor.h"
#include "../Interfaces/IListCommandBuilder.h"
#include "../Interfaces/ILaserControl.h"
#include "../Rtc6Common.h"
#include <vector>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace RTC6 {

/**
 * @brief Implementation of IMicroVectorProcessor for RTC6 controller
 * 
 * This class processes geometric paths into micro-vector commands with support for
 * dynamic laser parameter adjustment and path optimization.
 */
class MicroVectorProcessor : public IMicroVectorProcessor {
public:
    /**
     * @brief Construct a new MicroVectorProcessor
     * @param listBuilder The command builder for adding micro-vector commands
     * @param laserControl The controller for laser parameter management
     * @throws std::invalid_argument if either pointer is null
     */
    explicit MicroVectorProcessor(
        IListCommandBuilder* listBuilder = nullptr,
        ILaserControl* laserControl = nullptr
    );

    // Disable copy and move
    MicroVectorProcessor(const MicroVectorProcessor&) = delete;
    MicroVectorProcessor& operator=(const MicroVectorProcessor&) = delete;
    MicroVectorProcessor(MicroVectorProcessor&&) = delete;
    MicroVectorProcessor& operator=(MicroVectorProcessor&&) = delete;

    // IMicroVectorProcessor implementation
    void ProcessMicroVectorPath(
        const std::vector<Point>& path,
        IListCommandBuilder* listBuilder = nullptr,
        ILaserControl* laserControl = nullptr
    ) override;

    // Configuration methods
    void SetTimeStep(double timeStep) override;
    double GetTimeStep() const override;
    void SetMinimumSegmentLength(int minLength) override;
    int GetMinimumSegmentLength() const override;
    void EnableDynamicParameterAdjustment(bool enable) override;
    bool IsDynamicParameterAdjustmentEnabled() const override;

    // Factory method implementation
    static Ptr Create(IListCommandBuilder* listBuilder, ILaserControl* laserControl);

private:
    // Helper methods
    double calculateSpeedForSegment(const Point& p1, const Point& p2, double defaultSpeed) const;
    void processSegment(
        const Point& start,
        const Point& end,
        IListCommandBuilder* builder,
        ILaserControl* laser
    ) const;

    // Member variables
    IListCommandBuilder* listBuilder_;
    ILaserControl* laserControl_;
    double timeStep_ = 10.0;  // Default 10µs time step
    int minSegmentLength_ = 10;  // Minimum segment length in bits
    bool dynamicParamsEnabled_ = true;  // Enable dynamic parameter adjustment

    // Static helper methods
    static double calculateDistance(const Point& p1, const Point& p2);
    static Point calculateStep(const Point& start, const Point& end, int totalSteps, int currentStep);
    static int calculateNumberOfMicroVectors(double distance, double speed, double timeStep);
};

} // namespace RTC6

#endif // MICRO_VECTOR_PROCESSOR_H
