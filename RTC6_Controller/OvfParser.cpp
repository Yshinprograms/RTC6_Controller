#include "OvfParser.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <cerrno>

// Official Protobuf utilities
#include <google/protobuf/util/delimited_message_util.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

// Generated Protobuf headers
#include "open_vector_format.pb.h"
#include "ovf_lut.pb.h"

// The public method signature now matches the header file
open_vector_format::Job OvfParser::parseFile(const std::string& filePath) {
    std::cout << "[OvfParser] Starting to parse file: " << filePath << std::endl;
    open_vector_format::Job job; // This is the object we will build and return.

    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "FATAL: [OvfParser] Cannot open file stream. System error: ";
#ifdef _MSC_VER
        char err_buf[256];
        if (strerror_s(err_buf, sizeof(err_buf), errno) == 0) {
            std::cerr << err_buf;
        }
        else {
            std::cerr << "Unknown error";
        }
#else
        std::cerr << strerror(errno);
#endif
        std::cerr << std::endl;
        return job; // Return empty job
    }

    // 1. Verify Magic Number
    char magic[4];
    file.read(magic, 4);
    if (!file.good() || file.gcount() != 4 || magic[0] != 0x4c || magic[1] != 0x56 || magic[2] != 0x46 || magic[3] != 0x21) {
        std::cerr << "FATAL: [OvfParser] File magic number mismatch or read error." << std::endl;
        return job;
    }
    std::cout << "[OvfParser] Magic number verified." << std::endl;

    // 2. Read JobLUT position
    int64_t job_lut_position;
    file.read(reinterpret_cast<char*>(&job_lut_position), sizeof(job_lut_position));
    if (!file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to read JobLUT position pointer." << std::endl;
        return job;
    }
    std::cout << "[OvfParser] JobLUT pointer is: 0x" << std::hex << job_lut_position << std::dec << std::endl;

    // 3. Parse JobLUT
    file.seekg(job_lut_position);
    if (!file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to seek to JobLUT position." << std::endl;
        return job;
    }
    google::protobuf::io::IstreamInputStream zero_copy_input_job_lut(&file);
    open_vector_format::JobLUT job_lut; // <-- FIX: Declared here to be in scope
    if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&job_lut, &zero_copy_input_job_lut, nullptr)) {
        std::cerr << "FATAL: [OvfParser] Failed to parse JobLUT." << std::endl;
        return job;
    }
    std::cout << "[OvfParser] JobLUT parsed successfully." << std::endl;

    // 4. Parse JobShell and store it in our return object
    file.clear();
    file.seekg(job_lut.jobshellposition());
    if (!file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to seek to JobShell position." << std::endl;
        return job;
    }
    google::protobuf::io::IstreamInputStream zero_copy_input_shell(&file);
    if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&job, &zero_copy_input_shell, nullptr)) {
        std::cerr << "ERROR: [OvfParser] Failed to parse JobShell." << std::endl;
        return {};
    }
    std::cout << "[OvfParser] JobShell parsed successfully. Job Name: " << job.job_meta_data().job_name() << std::endl;

    // 5. Iterate through WorkPlanes and load their full data into the job object
    std::cout << "[OvfParser] Loading geometry for " << job_lut.workplanepositions_size() << " work planes..." << std::endl;
    for (int i = 0; i < job_lut.workplanepositions_size(); ++i) {
        file.clear();
        file.seekg(job_lut.workplanepositions(i));
        int64_t wp_lut_position;
        file.read(reinterpret_cast<char*>(&wp_lut_position), sizeof(wp_lut_position));

        file.clear();
        file.seekg(wp_lut_position);
        google::protobuf::io::IstreamInputStream zero_copy_input_wp_lut(&file);
        open_vector_format::WorkPlaneLUT wp_lut;
        google::protobuf::util::ParseDelimitedFromZeroCopyStream(&wp_lut, &zero_copy_input_wp_lut, nullptr);

        // Add a new WorkPlane to our job's list
        open_vector_format::WorkPlane* work_plane = job.add_work_planes();

        // Parse the WorkPlaneShell directly into the newly added WorkPlane
        file.clear();
        file.seekg(wp_lut.workplaneshellposition());
        google::protobuf::io::IstreamInputStream zero_copy_input_wp_shell(&file);
        google::protobuf::util::ParseDelimitedFromZeroCopyStream(work_plane, &zero_copy_input_wp_shell, nullptr);

        // Iterate and parse each VectorBlock, adding it to the current work_plane
        for (int j = 0; j < wp_lut.vectorblockspositions_size(); ++j) {
            file.clear();
            file.seekg(wp_lut.vectorblockspositions(j));
            google::protobuf::io::IstreamInputStream zero_copy_input_vb(&file);

            // Add a new VectorBlock to the WorkPlane and parse the data into it
            google::protobuf::util::ParseDelimitedFromZeroCopyStream(work_plane->add_vector_blocks(), &zero_copy_input_vb, nullptr);
        }
    }

    std::cout << "[OvfParser] Parsing complete. Job has " << job.work_planes_size() << " layers." << std::endl;
    return job;
}