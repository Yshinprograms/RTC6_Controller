#pragma once

#include <string>
#include <fstream>
#include <vector>

#include "open_vector_format.pb.h"
#include "ovf_lut.pb.h"

class OvfParser {
public:
    OvfParser();
    ~OvfParser();

    // --- Public API ---
    // These methods remain unchanged as they form the public contract of the class.
    bool openFile(const std::string& filePath);
    int getNumberOfWorkPlanes() const;
    open_vector_format::Job getJobShell() const;
    open_vector_format::WorkPlane getWorkPlane(int index);

private:
    // --- Private Helper Methods (SLAP Refactoring) ---

    // Top-level helpers for openFile()
    bool readAndValidateHeader(int64_t& out_jobLutPos);
    bool parseMasterLut(int64_t jobLutPos);
    bool parseJobShell();
    bool parseAllWorkPlaneLuts();

    // Generic, low-level helper for parsing any Protobuf message from a specific file position.
    template <typename T>
    bool parseDelimitedMessageAt(T* message, int64_t position);

    // Private Member Variables
    std::ifstream m_file;
    open_vector_format::Job m_jobShell;
    open_vector_format::JobLUT m_jobLut;
    std::vector<open_vector_format::WorkPlaneLUT> m_workPlaneLuts;
};