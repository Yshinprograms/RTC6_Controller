#include "OvfParser.h"
#include <iostream>
#include <stdexcept>

#include <google/protobuf/util/delimited_message_util.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

OvfParser::OvfParser() {}

OvfParser::~OvfParser() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

// =================================================================================
// === PUBLIC METHODS (HIGH LEVEL OF ABSTRACTION) ==================================
// =================================================================================

/**
 * @brief The refactored openFile method.
 * It now reads like a high-level summary of the opening process, delegating
 * all low-level work to private helper methods. This is SLAP in action.
 */
bool OvfParser::openFile(const std::string& filePath) {
    std::cout << "[OvfParser] LAZY OPEN: Opening file: " << filePath << std::endl;

    if (m_file.is_open()) {
        m_file.close();
    }
    m_jobShell.Clear();
    m_jobLut.Clear();
    m_workPlaneLuts.clear();

    m_file.open(filePath, std::ios::in | std::ios::binary);
    if (!m_file.is_open()) {
        std::cerr << "FATAL: [OvfParser] Cannot open file stream." << std::endl;
        return false;
    }

    int64_t jobLutPosition = 0;
    if (!readAndValidateHeader(jobLutPosition)) return false;
    if (!parseMasterLut(jobLutPosition)) return false;
    if (!parseJobShell()) return false;
    if (!parseAllWorkPlaneLuts()) return false;

    std::cout << "[OvfParser] LAZY OPEN complete. Ready to load geometry on demand." << std::endl;
    return true;
}

/**
 * @brief The refactored getWorkPlane method.
 * It focuses on the logic of getting the layer, while the generic, low-level
 * "seek-and-parse" operation is handled by a reusable helper.
 */
open_vector_format::WorkPlane OvfParser::getWorkPlane(int index) {
    if (!m_file.is_open()) {
        throw std::runtime_error("File is not open. Call openFile() first.");
    }
    if (index < 0 || index >= m_workPlaneLuts.size()) {
        throw std::out_of_range("WorkPlane index is out of range.");
    }

    std::cout << "[OvfParser] LAZY LOAD: Loading full geometry for WorkPlane " << index << std::endl;

    const auto& wp_lut = m_workPlaneLuts.at(index);
    open_vector_format::WorkPlane work_plane;

    // Use the generic helper to parse the main shell.
    if (!parseDelimitedMessageAt(&work_plane, wp_lut.workplaneshellposition())) {
        throw std::runtime_error("Failed to parse WorkPlaneShell for index " + std::to_string(index));
    }

    // Use the generic helper again to parse the vector blocks.
    for (int j = 0; j < wp_lut.vectorblockspositions_size(); ++j) {
        if (!parseDelimitedMessageAt(work_plane.add_vector_blocks(), wp_lut.vectorblockspositions(j))) {
            // In a production tool, you might want to just log a warning and continue.
            throw std::runtime_error("Failed to parse VectorBlock " + std::to_string(j));
        }
    }

    std::cout << "[OvfParser] LAZY LOAD complete. Loaded " << work_plane.vector_blocks_size() << " vector blocks." << std::endl;
    return work_plane;
}

int OvfParser::getNumberOfWorkPlanes() const {
    // This was already simple and correct.
    return static_cast<int>(m_workPlaneLuts.size());
}

open_vector_format::Job OvfParser::getJobShell() const {
    // This was already simple and correct.
    return m_jobShell;
}


// =================================================================================
// === PRIVATE HELPER METHODS (LOWER LEVEL OF ABSTRACTION) =========================
// =================================================================================

bool OvfParser::readAndValidateHeader(int64_t& out_jobLutPos) {
    // 1. Read and validate magic number
    char magic[4];
    m_file.read(magic, 4);
    if (m_file.gcount() != 4 || (magic[0] != 0x4c || magic[1] != 0x56 || magic[2] != 0x46 || magic[3] != 0x21)) {
        std::cerr << "FATAL: [OvfParser] File magic number mismatch or read error." << std::endl;
        return false;
    }

    // 2. Read the JobLUT position pointer
    m_file.read(reinterpret_cast<char*>(&out_jobLutPos), sizeof(out_jobLutPos));
    if (m_file.gcount() != sizeof(out_jobLutPos)) {
        std::cerr << "FATAL: [OvfParser] Failed to read JobLUT position pointer." << std::endl;
        return false;
    }
    return true;
}

bool OvfParser::parseMasterLut(int64_t jobLutPos) {
    if (!parseDelimitedMessageAt(&m_jobLut, jobLutPos)) {
        std::cerr << "FATAL: [OvfParser] Failed to parse JobLUT." << std::endl;
        return false;
    }
    std::cout << "[OvfParser] JobLUT parsed successfully." << std::endl;
    return true;
}

bool OvfParser::parseJobShell() {
    if (!parseDelimitedMessageAt(&m_jobShell, m_jobLut.jobshellposition())) {
        std::cerr << "ERROR: [OvfParser] Failed to parse JobShell." << std::endl;
        return false;
    }
    std::cout << "[OvfParser] JobShell parsed. Job Name: " << m_jobShell.job_meta_data().job_name() << std::endl;
    return true;
}

bool OvfParser::parseAllWorkPlaneLuts() {
    std::cout << "[OvfParser] Reading " << m_jobLut.workplanepositions_size() << " WorkPlane LUTs..." << std::endl;
    m_workPlaneLuts.reserve(m_jobLut.workplanepositions_size());

    for (int i = 0; i < m_jobLut.workplanepositions_size(); ++i) {
        // First, read the pointer to the actual LUT data
        int64_t wp_lut_position = 0;
        m_file.clear();
        m_file.seekg(m_jobLut.workplanepositions(i));
        m_file.read(reinterpret_cast<char*>(&wp_lut_position), sizeof(wp_lut_position));
        if (m_file.gcount() != sizeof(wp_lut_position)) {
            std::cerr << "ERROR: [OvfParser] Failed to read WorkPlaneLUT pointer for layer " << i << std::endl;
            return false;
        }

        // Now, parse the LUT from that position
        open_vector_format::WorkPlaneLUT wp_lut;
        if (!parseDelimitedMessageAt(&wp_lut, wp_lut_position)) {
            std::cerr << "ERROR: [OvfParser] Failed to parse WorkPlaneLUT for layer " << i << std::endl;
            return false;
        }
        m_workPlaneLuts.push_back(wp_lut);
    }
    return true;
}

/**
 * @brief A generic, low-level helper for all file parsing.
 * This template function can parse any Protobuf message type from a given
 * position in the file, handling the necessary stream operations.
 */
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