#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <google/protobuf/util/delimited_message_util.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "open_vector_format.pb.h"
#include "ovf_lut.pb.h"

class OvfParser {
public:
    OvfParser();
    ~OvfParser();

    // --- Public API ---
    bool openFile(const std::string& filePath);
    int getNumberOfWorkPlanes() const;
    open_vector_format::Job getJobShell() const;
    open_vector_format::WorkPlane getWorkPlane(int index);

private:
    // --- Private Helper Methods ---

    // Top-level helpers for openFile()
    bool readAndValidateHeader(int64_t& out_jobLutPos);
    bool parseMasterLut(int64_t jobLutPos);
    bool parseJobShell();
    bool parseAllWorkPlaneLuts();

    // Top-level helpers for getWorkPlane()
    bool parseWorkPlaneShell(const open_vector_format::WorkPlaneLUT& lut, open_vector_format::WorkPlane* out_plane);
    bool parseVectorBlocks(const open_vector_format::WorkPlaneLUT& lut, open_vector_format::WorkPlane* out_plane);

    // Generic, low-level helpers
    template <typename T>
    bool parseDelimitedMessageAt(T* message, int64_t position);

    template <typename T>
    bool parseMessageFromPointerAt(T* message, int64_t pointerPosition);

    // --- Private Member Variables ---
    std::ifstream m_file;
    open_vector_format::Job m_jobShell;
    open_vector_format::JobLUT m_jobLut;
    std::vector<open_vector_format::WorkPlaneLUT> m_workPlaneLuts;
};


// =================================================================================
// === TEMPLATE FUNCTION DEFINITIONS (MUST BE IN HEADER) ===========================
// =================================================================================

template <typename T>
bool OvfParser::parseDelimitedMessageAt(T* message, int64_t position) {
    m_file.clear();
    m_file.seekg(position);
    if (!m_file.good()) {
        return false;
    }

    google::protobuf::io::IstreamInputStream zero_copy_input(&m_file);
    return google::protobuf::util::ParseDelimitedFromZeroCopyStream(message, &zero_copy_input, nullptr);
}

template <typename T>
bool OvfParser::parseMessageFromPointerAt(T* message, int64_t pointerPosition) {
    int64_t dataPosition = 0;
    m_file.clear();
    m_file.seekg(pointerPosition);
    m_file.read(reinterpret_cast<char*>(&dataPosition), sizeof(dataPosition));
    if (m_file.gcount() != sizeof(dataPosition)) {
        return false;
    }
    return parseDelimitedMessageAt(message, dataPosition);
}