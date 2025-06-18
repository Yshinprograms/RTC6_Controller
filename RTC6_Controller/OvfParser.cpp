#include "OvfParser.h"
#include <iostream>
#include <stdexcept>

OvfParser::OvfParser() {}

OvfParser::~OvfParser() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

// =================================================================================
// === PUBLIC METHODS ==============================================================
// =================================================================================

bool OvfParser::openFile(const std::string& filePath) {
    if (m_file.is_open()) {
        m_file.close();
    }
    m_jobShell.Clear();
    m_jobLut.Clear();
    m_workPlaneLuts.clear();

    m_file.open(filePath, std::ios::in | std::ios::binary);
    if (!m_file.is_open()) {
        return false;
    }

    int64_t jobLutPosition = 0;
    if (!readAndValidateHeader(jobLutPosition)) return false;
    if (!parseMasterLut(jobLutPosition)) return false;
    if (!parseJobShell()) return false;
    if (!parseAllWorkPlaneLuts()) return false;

    return true;
}

open_vector_format::WorkPlane OvfParser::getWorkPlane(int index) {
    if (!m_file.is_open()) {
        throw std::runtime_error("File is not open. Call openFile() first.");
    }
    if (index < 0 || index >= m_workPlaneLuts.size()) {
        throw std::out_of_range("WorkPlane index is out of range.");
    }

    const auto& wp_lut = m_workPlaneLuts.at(index);
    open_vector_format::WorkPlane work_plane;

    if (!parseWorkPlaneShell(wp_lut, &work_plane)) {
        throw std::runtime_error("Failed to parse WorkPlaneShell for index " + std::to_string(index));
    }
    if (!parseVectorBlocks(wp_lut, &work_plane)) {
        throw std::runtime_error("Failed to parse VectorBlocks for index " + std::to_string(index));
    }

    return work_plane;
}

int OvfParser::getNumberOfWorkPlanes() const {
    return static_cast<int>(m_workPlaneLuts.size());
}

open_vector_format::Job OvfParser::getJobShell() const {
    return m_jobShell;
}

// =================================================================================
// === PRIVATE HELPER METHODS ======================================================
// =================================================================================

bool OvfParser::readAndValidateHeader(int64_t& out_jobLutPos) {
    char magic[4];
    m_file.read(magic, 4);
    if (m_file.gcount() != 4 || (magic[0] != 0x4c || magic[1] != 0x56 || magic[2] != 0x46 || magic[3] != 0x21)) {
        return false;
    }
    m_file.read(reinterpret_cast<char*>(&out_jobLutPos), sizeof(out_jobLutPos));
    return m_file.gcount() == sizeof(out_jobLutPos);
}

bool OvfParser::parseMasterLut(int64_t jobLutPos) {
    return parseDelimitedMessageAt(&m_jobLut, jobLutPos);
}

bool OvfParser::parseJobShell() {
    return parseDelimitedMessageAt(&m_jobShell, m_jobLut.jobshellposition());
}

bool OvfParser::parseAllWorkPlaneLuts() {
    m_workPlaneLuts.reserve(m_jobLut.workplanepositions_size());
    for (int i = 0; i < m_jobLut.workplanepositions_size(); ++i) {
        open_vector_format::WorkPlaneLUT wp_lut;
        if (!parseMessageFromPointerAt(&wp_lut, m_jobLut.workplanepositions(i))) {
            return false;
        }
        m_workPlaneLuts.push_back(wp_lut);
    }
    return true;
}

bool OvfParser::parseWorkPlaneShell(const open_vector_format::WorkPlaneLUT& lut, open_vector_format::WorkPlane* out_plane) {
    return parseDelimitedMessageAt(out_plane, lut.workplaneshellposition());
}

bool OvfParser::parseVectorBlocks(const open_vector_format::WorkPlaneLUT& lut, open_vector_format::WorkPlane* out_plane) {
    for (int j = 0; j < lut.vectorblockspositions_size(); ++j) {
        if (!parseDelimitedMessageAt(out_plane->add_vector_blocks(), lut.vectorblockspositions(j))) {
            return false;
        }
    }
    return true;
}