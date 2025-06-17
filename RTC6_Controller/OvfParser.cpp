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

// Prepares the parser for lazy loading by reading the file's "Table of Contents".
// This function opens the specified OVF file and reads all metadata:
// - The main JobLUT (the master list of where everything is)
// - The JobShell (the "glossary" of all available laser parameters)
// - The LUT for each individual WorkPlane (layer)
// (JobLUT)
// It does NOT read any of the heavy vector geometry. It stores the metadata in member
// variables and returns true on success, priming the parser for getWorkPlane() calls.
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

	// Magic number checked for first 4 bytes to ensure it's a valid OVF file.
    char magic[4];
	// Read the first 4 bytes and shifts the file pointer to the next position (lut_position)
    m_file.read(magic, 4);
    if (!m_file.good() || (magic[0] != 0x4c || magic[1] != 0x56 
        || magic[2] != 0x46 || magic[3] != 0x21)) {
        std::cerr << "FATAL: [OvfParser] File magic number mismatch or read error." << std::endl;
        return false;
    }

    int64_t job_lut_position;
	// read(&copy_into_this_memory_address, amount_of_bytes_to_copy) starts copying the data it
	// finds at the address it is pointing to, into another destination(&job_lut_position).
    // 
    // The read() function is a low-level tool. It moves raw bytes from the file (the source)
    // into a memory address (the destination). To do this safely, it needs two things:
    //   1. The destination's memory address, disguised as a raw byte pointer (char*).
    //   2. The exact number of bytes to copy.
    //
    // The reinterpret_cast is the disguise. It tells the compiler: "For this one function call,
    // pretend the memory address of my 'int64_t' variable is just a generic pointer to a
    // bucket of bytes." This makes the read() function happy, and it copies the next 8 bytes
    // from the file directly into the memory of job_lut_position.
    m_file.read(reinterpret_cast<char*>(&job_lut_position), sizeof(job_lut_position));
    if (!m_file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to read JobLUT position pointer." << std::endl;
        return false;
    }

	// Seek Get the JobLUT position from our forceful interpretation
	// Then use IstreamInputStream to parse the JobLUT & load it into m_jobLut
    m_file.seekg(job_lut_position);
	// Use a local scope to create a temporary wrapper to ensure buffer is cleared after use
    {
		// Protobuf helper to wrap the file stream for parsing
        google::protobuf::io::IstreamInputStream zero_copy_input(&m_file);
		// Check if successfully: Read stream > Copy into m_jobLut > use zero_copy_input
        if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&m_jobLut, &zero_copy_input, nullptr)) {
            std::cerr << "FATAL: [OvfParser] Failed to parse JobLUT." << std::endl;
            return false;
        }
    }
    std::cout << "[OvfParser] JobLUT parsed successfully." << std::endl;

    // --- Parse JobShell ---
    m_file.clear(); // CRITICAL: Clear flags after any read operation to re-read file
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

/* 
* Fetches the complete data for one single layer("WorkPlane") from the file.
* This is the "heavy-lifting" part of the lazy-loading pattern. It gets called
* repeatedly inside the main processing loop.
*
* Using the layer's index, it finds the layer's own Look-Up Table that was loaded
* by openFile(). From that LUT, it gets the exact file positions for the layer's
* shell and all of its geometry blocks. It then seeks to those positions in the
* file and parses the data. (WorkPlaneLUT)
*
* @param index The zero-based index of the layer you want to load (e.g., 0, 1, 2...).
* @return A fully-populated open_vector_format::WorkPlane object with all its geometry.
* @throws std::out_of_range if the index is invalid, or std::runtime_error if file operations fail.
*/
open_vector_format::WorkPlane OvfParser::getWorkPlane(int index) {
    if (!m_file.is_open()) {
        throw std::runtime_error("File is not open. Call openFile() first.");
    }

    std::cout << "[OvfParser] LAZY LOAD: Loading full geometry for WorkPlane " << index << std::endl;
    if (index < 0 || index >= m_workPlaneLuts.size()) {
        throw std::out_of_range("WorkPlane index is out of range.");
    }

    const auto& wp_lut = m_workPlaneLuts[index];
    open_vector_format::WorkPlane work_plane;
    m_file.clear();
    m_file.seekg(wp_lut.workplaneshellposition());
    if (!m_file.good()) {
        throw std::runtime_error("Failed to seek to WorkPlaneShell position for index " 
            + std::to_string(index));
    }
    {
        google::protobuf::io::IstreamInputStream zero_copy_input(&m_file);
        if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&work_plane, &zero_copy_input, nullptr)) {
            throw std::runtime_error("Failed to parse WorkPlaneShell for index " 
                + std::to_string(index));
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
                throw std::runtime_error("Failed to parse VectorBlock " 
                    + std::to_string(j) + " in WorkPlane " + std::to_string(index));
            }
        }
    }
    std::cout << "[OvfParser] LAZY LOAD complete. Loaded " 
        << work_plane.vector_blocks_size() << " vector blocks." << std::endl;
    return work_plane;
}