#pragma once
#include <string>
#include "open_vector_format.pb.h"

class InterfaceOvfParser {
public:
    virtual ~InterfaceOvfParser() = default;

    /**
     * @brief Opens and parses the metadata of an OVF file.
     * @param filePath The path to the .ovf file.
     * @return True if the file was opened and parsed successfully, false otherwise.
     */
    virtual bool openFile(const std::string& filePath) = 0;

    /**
     * @brief Gets the total number of work planes (layers) in the opened file.
     * @return The number of work planes.
     */
    virtual int getNumberOfWorkPlanes() const = 0;

    /**
     * @brief Gets the top-level Job "shell" containing metadata and parameter maps.
     * @return The parsed Job message.
     */
    virtual open_vector_format::Job getJobShell() const = 0;

    /**
     * @brief Lazily loads and returns the complete data for a single work plane.
     * @param index The zero-based index of the work plane to retrieve.
     * @return The complete WorkPlane message with all its vector blocks.
     */
    virtual open_vector_format::WorkPlane getWorkPlane(int index) = 0;
};