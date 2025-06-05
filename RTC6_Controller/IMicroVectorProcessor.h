// Interfaces/IMicroVectorProcessor.h
#ifndef I_MICRO_VECTOR_PROCESSOR_H
#define I_MICRO_VECTOR_PROCESSOR_H

#include <vector>
#include <memory>    // For std::unique_ptr
#include "Rtc6Common.h" // For Point, UINT, LONG types

// Forward declarations
class IListCommandBuilder;
class ILaserControl;

// Interface for processing geometric paths into micro-vector commands.
// This interface defines the contract for classes that convert high-level
// geometric paths (series of points) into low-level micro-vector commands
// for the RTC6 controller, enabling fine-grained control.
class IMicroVectorProcessor {
public:
    // Using std::unique_ptr for clear ownership.
    using Ptr = std::unique_ptr<IMicroVectorProcessor>;

    virtual ~IMicroVectorProcessor() = default;

    // ===== Path Processing =====

    /**
     * @brief Process a path of points into micro-vector commands and add them to the list.
     * The list builder and laser control dependencies are expected to be injected via constructor.
     * @param path The sequence of points defining the path to be marked.
     * @throws std::invalid_argument if path has less than 2 points.
     */
    virtual void ProcessMicroVectorPath(const std::vector<Point>& path) = 0;

    // ===== Configuration =====

    /**
     * @brief Set the time step for each individual micro-vector segment (in microseconds).
     * This affects the effective speed if total distance is fixed.
     * RTC6 micro-vectors are typically 10us steps.
     * @param timeStep Time step in microseconds (e.g., 10.0 for 10us).
     * @throws std::out_of_range if timeStep is outside a reasonable range (e.g., < 1us or > 1ms).
     */
    virtual void SetMicroVectorTimeStep(double timeStep) = 0;

    /**
     * @brief Get the current time step for micro-vector generation.
     * @return double Time step in microseconds.
     */
    virtual double GetMicroVectorTimeStep() const = 0;

    /**
     * @brief Set the minimum segment length (in RTC6 bits) to be processed.
     * Segments shorter than this may be skipped or handled differently.
     * @param minLength Minimum segment length in bits.
     * @throws std::invalid_argument if minLength is negative.
     */
    virtual void SetMinimumSegmentLength(LONG minLength) = 0;

    /**
     * @brief Get the minimum segment length for path simplification.
     * @return LONG Minimum segment length in bits.
     */
    virtual LONG GetMinimumSegmentLength() const = 0;

    /**
     * @brief Enable or disable dynamic laser parameter adjustment at each segment.
     * When enabled, `SetCurrentListLaserPower`, `SetCurrentListMarkSpeed`, etc. are called.
     * @param enable True to enable dynamic adjustment, false to use fixed parameters.
     */
    virtual void EnableDynamicParameterAdjustment(bool enable) = 0;

    /**
     * @brief Check if dynamic laser parameter adjustment is enabled.
     * @return bool True if dynamic adjustment is enabled.
     */
    virtual bool IsDynamicParameterAdjustmentEnabled() const = 0;
};

#endif // I_MICRO_VECTOR_PROCESSOR_H