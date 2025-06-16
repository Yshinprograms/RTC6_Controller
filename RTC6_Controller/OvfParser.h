#pragma once
#include <string>
#include <fstream>
#include <vector>

// Generated Protobuf headers
#include "open_vector_format.pb.h"
#include "ovf_lut.pb.h"

class OvfParser {
public:
    OvfParser();
    ~OvfParser();

    // The Lazy Loading Interface:
    // --------------------------

    // Step 1: Opens the file, reads all LUTs, but no geometry.
    // Returns true on success, false on failure.
    bool openFile(const std::string& filePath);

    // Step 2 (Optional): Returns a copy of the high-level Job metadata.
    open_vector_format::Job getJobShell() const;

    // Step 3: Parses and returns the FULL data for a single WorkPlane on demand.
    open_vector_format::WorkPlane getWorkPlane(int index);

	int getNumberOfWorkPlanes() const;

private:
    std::ifstream m_file;

    // Member variables to store the state read by openFile()
    open_vector_format::Job m_jobShell;
    open_vector_format::JobLUT m_jobLut;
    std::vector<open_vector_format::WorkPlaneLUT> m_workPlaneLuts;
};