#ifndef I_MICRO_VECTOR_PROCESSOR_H
#define I_MICRO_VECTOR_PROCESSOR_H

#include <vector>
#include <memory>
#include "Rtc6Common.h"

// Forward declarations
class IListCommandBuilder;
class ILaserControl;

/**
 * @brief Interface for processing geometric paths into micro-vector commands
 * 
 * This interface defines the contract for classes that convert high-level
 * geometric paths into low-level micro-vector commands for the RTC6 controller.
 * It supports both 2D and 3D motion control with dynamic laser parameter adjustment.
 */
class IMicroVectorProcessor {
public:
    using Ptr = std::shared_ptr<IMicroVectorProcessor>;

    virtual ~IMicroVectorProcessor() = default;
    
    // ===== Path Processing =====
    
    /**
     * @brief Process a path of points into micro-vector commands
     * @param path The sequence of points defining the path to be marked
     * @param listBuilder The command builder for adding micro-vector commands
     * @param laserControl The controller for laser parameter management
     * @throws std::invalid_argument if path has less than 2 points or null pointers
     */
    virtual void ProcessMicroVectorPath(
        const std::vector<Point>& path,
        IListCommandBuilder* listBuilder = nullptr,
        ILaserControl* laserControl = nullptr
    ) = 0;
    
    // ===== Configuration =====
    
    /**
     * @brief Set the time step for micro-vector generation (in microseconds)
     * @param timeStep Time step in microseconds (default: 10µs)
     */
    virtual void SetTimeStep(double timeStep) = 0;
    
    /**
     * @brief Get the current time step for micro-vector generation
     * @return double Time step in microseconds
     */
    virtual double GetTimeStep() const = 0;
    
    /**
     * @brief Set the minimum segment length for path simplification
     * @param minLength Minimum segment length in bits
     */
    virtual void SetMinimumSegmentLength(int minLength) = 0;
    
    /**
     * @brief Get the minimum segment length for path simplification
     * @return int Minimum segment length in bits
     */
    virtual int GetMinimumSegmentLength() const = 0;
    
    /**
     * @brief Enable or disable dynamic laser parameter adjustment
     * @param enable True to enable dynamic adjustment, false to use fixed parameters
     */
    virtual void EnableDynamicParameterAdjustment(bool enable) = 0;
    
    /**
     * @brief Check if dynamic laser parameter adjustment is enabled
     * @return bool True if dynamic adjustment is enabled
     */
    virtual bool IsDynamicParameterAdjustmentEnabled() const = 0;
    
    // ===== Factory Method =====
    
    /**
     * @brief Create a new instance of the default micro-vector processor
     * @param listBuilder The list command builder to use
     * @param laserControl The laser controller to use
     * @return std::shared_ptr<IMicroVectorProcessor> Shared pointer to the new processor
     */
    static Ptr Create(IListCommandBuilder* listBuilder, ILaserControl* laserControl);
};

#endif // I_MICRO_VECTOR_PROCESSOR_H
