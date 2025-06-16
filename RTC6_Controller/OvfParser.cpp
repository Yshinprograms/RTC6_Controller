#include "OvfParser.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

// Official Protobuf utilities
#include <google/protobuf/util/delimited_message_util.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

OvfParser::OvfParser() {}

OvfParser::~OvfParser() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

bool OvfParser::openFile(const std::string& filePath) {
    std::cout << "[OvfParser] LAZY OPEN: Opening file and reading LUTs: " << filePath << std::endl;
    if (m_file.is_open()) {
        m_file.close();
    }
    m_jobShell.Clear();
    m_jobLut.Clear();
    m_workPlaneLuts.clear();

    m_file.open(filePath, std::ios::in | std::ios::binary);
    if (!m_file) {
        std::cerr << "FATAL: [OvfParser] Cannot open file stream." << std::endl;
        return false;
    }

    // Magic number and LUT position are read directly.
    char magic[4];
    m_file.read(magic, 4);
    if (!m_file.good() || /* ... checks ... */ (magic[0] != 0x4c || magic[1] != 0x56 || magic[2] != 0x46 || magic[3] != 0x21)) {
        std::cerr << "FATAL: [OvfParser] File magic number mismatch or read error." << std::endl;
        return false;
    }
    int64_t job_lut_position;
    m_file.read(reinterpret_cast<char*>(&job_lut_position), sizeof(job_lut_position));
    if (!m_file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to read JobLUT position pointer." << std::endl;
        return false;
    }

    // --- Parse JobLUT ---
    m_file.seekg(job_lut_position);
    { // Use a scope to create a temporary wrapper.
        google::protobuf::io::IstreamInputStream zero_copy_input(&m_file);
        if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&m_jobLut, &zero_copy_input, nullptr)) {
            std::cerr << "FATAL: [OvfParser] Failed to parse JobLUT." << std::endl;
            return false;
        }
    }
    std::cout << "[OvfParser] JobLUT parsed successfully." << std::endl;

    // --- Parse JobShell ---
    m_file.clear(); // CRITICAL: Clear flags after any read operation.
    m_file.seekg(m_jobLut.jobshellposition());
    {
        google::protobuf::io::IstreamInputStream zero_copy_input(&m_file);
        if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&m_jobShell, &zero_copy_input, nullptr)) {
            std::cerr << "ERROR: [OvfParser] Failed to parse JobShell." << std::endl;
            return false;
        }
    }
    std::cout << "[OvfParser] JobShell parsed. Job Name: " << m_jobShell.job_meta_data().job_name() << std::endl;

    // --- Iterate and parse ONLY the WorkPlaneLUTs ---
    std::cout << "[OvfParser] Reading " << m_jobLut.workplanepositions_size() << " WorkPlane LUTs..." << std::endl;
    m_workPlaneLuts.reserve(m_jobLut.workplanepositions_size());
    for (int i = 0; i < m_jobLut.workplanepositions_size(); ++i) {
        m_file.clear();
        m_file.seekg(m_jobLut.workplanepositions(i));
        int64_t wp_lut_position;
        m_file.read(reinterpret_cast<char*>(&wp_lut_position), sizeof(wp_lut_position));
        if (!m_file.good()) {
            std::cerr << "ERROR: [OvfParser] Failed to read WorkPlaneLUT pointer for layer " << i << std::endl;
            return false;
        }

        m_file.clear();
        m_file.seekg(wp_lut_position);

        open_vector_format::WorkPlaneLUT wp_lut;
        {
            google::protobuf::io::IstreamInputStream zero_copy_input(&m_file);
            if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&wp_lut, &zero_copy_input, nullptr)) {
                std::cerr << "ERROR: [OvfParser] Failed to parse WorkPlaneLUT for layer " << i << std::endl;
                return false;
            }
        }
        m_workPlaneLuts.push_back(wp_lut);
    }
    std::cout << "[OvfParser] LAZY OPEN complete. Ready to load geometry on demand." << std::endl;
    return true;
}

int OvfParser::getNumberOfWorkPlanes() const {
    return m_jobLut.workplanepositions_size();
}

open_vector_format::Job OvfParser::getJobShell() const {
    return m_jobShell;
}

open_vector_format::WorkPlane OvfParser::getWorkPlane(int index) {
    // This function already used the correct pattern, so it remains unchanged.
    std::cout << "[OvfParser] LAZY LOAD: Loading full geometry for WorkPlane " << index << std::endl;
    if (index < 0 || index >= m_workPlaneLuts.size()) {
        throw std::out_of_range("WorkPlane index is out of range.");
    }
    if (!m_file.is_open()) {
        throw std::runtime_error("File is not open. Call openFile() first.");
    }
    const auto& wp_lut = m_workPlaneLuts[index];
    open_vector_format::WorkPlane work_plane;
    m_file.clear();
    m_file.seekg(wp_lut.workplaneshellposition());
    if (!m_file.good()) {
        throw std::runtime_error("Failed to seek to WorkPlaneShell position for index " + std::to_string(index));
    }
    {
        google::protobuf::io::IstreamInputStream zero_copy_input(&m_file);
        if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&work_plane, &zero_copy_input, nullptr)) {
            throw std::runtime_error("Failed to parse WorkPlaneShell for index " + std::to_string(index));
        }
    }
    for (int j = 0; j < wp_lut.vectorblockspositions_size(); ++j) {
        m_file.clear();
        m_file.seekg(wp_lut.vectorblockspositions(j));
        if (!m_file.good()) {
            throw std::runtime_error("Failed to seek to VectorBlock position " + std::to_string(j));
        }
        {
            google::protobuf::io::IstreamInputStream zero_copy_input(&m_file);
            if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(work_plane.add_vector_blocks(), &zero_copy_input, nullptr)) {
                throw std::runtime_error("Failed to parse VectorBlock " + std::to_string(j) + " in WorkPlane " + std::to_string(index));
            }
        }
    }
    std::cout << "[OvfParser] LAZY LOAD complete. Loaded " << work_plane.vector_blocks_size() << " vector blocks." << std::endl;
    return work_plane;
}