#pragma once
#include <string>

/**
 * @brief Defines the abstract contract for any class that provides user interface feedback.
 *
 * This interface allows the core PrintController logic to be completely decoupled
 * from the specific UI implementation (e.g., console, GUI, logger). The controller
 * only interacts with this contract.
 */
class InterfaceUI {
public:
    // Virtual destructor is essential for base classes in an inheritance hierarchy.
    virtual ~InterfaceUI() = default;

    /**
     * @brief Displays a standard, general-purpose message to the user.
     * @param message The content of the message to display.
     */
    virtual void displayMessage(const std::string& message) = 0;

    /**
     * @brief Displays an error message, typically formatted to stand out.
     * @param message The content of the error message.
     */
    virtual void displayError(const std::string& message) = 0;

    /**
     * @brief Displays progress for a long-running operation.
     * @param message A description of the current task (e.g., "Processing Layer").
     * @param current The current step number (e.g., 0-indexed layer number).
     * @param total The total number of steps.
     */
    virtual void displayProgress(const std::string& message, int current, int total) = 0;
};