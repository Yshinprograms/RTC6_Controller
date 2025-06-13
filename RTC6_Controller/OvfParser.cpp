#include "OvfParser.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string.h> // Required for strerror_s and strerror
#include <cerrno>   // Required for errno

// Official Protobuf utilities
#include <google/protobuf/util/delimited_message_util.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

// Generated Protobuf headers
#include "open_vector_format.pb.h"
#include "ovf_lut.pb.h"

constexpr double MAX_LASER_POWER_W = 100.0;

std::vector<OvfLayer> OvfParser::parseFile(const std::string& filePath) {
    std::cout << "[OvfParser] Starting to parse file: " << filePath << std::endl;
    std::vector<OvfLayer> allLayers;

    // 1. Open the file as a binary input stream
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file) {
        // --- CROSS-PLATFORM SAFE ERROR MESSAGE ---
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
        std::cerr << std::endl << "       Please check if the file exists at the specified path and that the program has read permissions." << std::endl;
        return allLayers;
    }

    // 2. Verify the 4-byte OVF Magic Number
    char magic[4];
    file.read(magic, 4);
    if (!file.good() || file.gcount() != 4) {
        std::cerr << "FATAL: [OvfParser] Failed to read magic number from file: " << filePath << std::endl;
        return allLayers;
    }
    if (magic[0] != 0x4c || magic[1] != 0x56 || magic[2] != 0x46 || magic[3] != 0x21) {
        std::cerr << "FATAL: [OvfParser] File is not a valid OVF file (magic number mismatch)." << std::endl;
        return allLayers;
    }
    std::cout << "[OvfParser] Magic number verified successfully." << std::endl;

    // 3. Read the 8-byte pointer to the main Look-Up Table (JobLUT)
    int64_t job_lut_position;
    file.read(reinterpret_cast<char*>(&job_lut_position), sizeof(job_lut_position));
    if (!file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to read JobLUT position pointer." << std::endl;
        return allLayers;
    }
    std::cout << "[OvfParser] JobLUT pointer is: 0x" << std::hex << job_lut_position << std::dec << std::endl;

    // 4. Seek to the JobLUT's position and parse it
    file.seekg(job_lut_position);
    if (!file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to seek to JobLUT position." << std::endl;
        return allLayers;
    }
    google::protobuf::io::IstreamInputStream zero_copy_input_job_lut(&file);
    open_vector_format::JobLUT job_lut;
    bool clean_eof = false;
    if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&job_lut, &zero_copy_input_job_lut, &clean_eof)) {
        std::cerr << "FATAL: [OvfParser] Failed to parse JobLUT. Reached EOF: " << (clean_eof ? "yes" : "no") << std::endl;
        return allLayers;
    }
    std::cout << "[OvfParser] JobLUT parsed successfully." << std::endl;
    std::cout << "  - JobShell Position: 0x" << std::hex << job_lut.jobshellposition() << std::dec << std::endl;
    std::cout << "  - Number of WorkPlane Pointers: " << job_lut.workplanepositions_size() << std::endl;


    // 5. Parse the JobShell
    file.clear();
    file.seekg(job_lut.jobshellposition());
    if (!file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to seek to JobShell position after clearing stream state." << std::endl;
        return allLayers;
    }
    google::protobuf::io::IstreamInputStream zero_copy_input_shell(&file);
    open_vector_format::Job job_shell;
    if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&job_shell, &zero_copy_input_shell, nullptr)) {
        std::cerr << "ERROR: [OvfParser] Failed to parse JobShell. The file might be corrupt at this location." << std::endl;
        return allLayers;
    }
    std::cout << "[OvfParser] JobShell parsed successfully. Job Name: " << job_shell.job_meta_data().job_name() << std::endl;

    // 6. Iterate through each WorkPlane and convert them back to your old data structure
    std::cout << "[OvfParser] Starting to process " << job_lut.workplanepositions_size() << " work planes..." << std::endl;
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

        file.clear();
        file.seekg(wp_lut.workplaneshellposition());
        google::protobuf::io::IstreamInputStream zero_copy_input_wp_shell(&file);
        open_vector_format::WorkPlane work_plane_shell;
        google::protobuf::util::ParseDelimitedFromZeroCopyStream(&work_plane_shell, &zero_copy_input_wp_shell, nullptr);

        OvfLayer currentLayer;
        currentLayer.z_height_mm = work_plane_shell.z_pos_in_mm();

        for (int j = 0; j < wp_lut.vectorblockspositions_size(); ++j) {
            file.clear();
            file.seekg(wp_lut.vectorblockspositions(j));
            google::protobuf::io::IstreamInputStream zero_copy_input_vb(&file);
            open_vector_format::VectorBlock vector_block;
            google::protobuf::util::ParseDelimitedFromZeroCopyStream(&vector_block, &zero_copy_input_vb, nullptr);

            if (vector_block.has_line_sequence()) {
                OvfPolyline currentPolyline;
                try {
                    const auto& ovf_params = job_shell.marking_params_map().at(vector_block.marking_params_key());
                    currentPolyline.params.markSpeed_mm_s = ovf_params.laser_speed_in_mm_per_s();
                    currentPolyline.params.focusOffset_mm = ovf_params.laser_focus_shift_in_mm();
                    currentPolyline.params.laserPowerPercent = (ovf_params.laser_power_in_w() / MAX_LASER_POWER_W) * 100.0;
                }
                catch (const std::out_of_range&) {
                    std::cerr << "WARNING: [OvfParser] Marking params key " << vector_block.marking_params_key()
                        << " not found. Using default parameters." << std::endl;
                }

                const auto& line_seq = vector_block.line_sequence();
                for (int p_idx = 0; p_idx < line_seq.points_size(); p_idx += 2) {
                    currentPolyline.points.push_back({
                        line_seq.points(p_idx),
                        line_seq.points(p_idx + 1),
                        currentLayer.z_height_mm
                        });
                }
                if (!currentPolyline.points.empty()) {
                    currentLayer.polylines.push_back(currentPolyline);
                }
            }
        }
        if (!currentLayer.polylines.empty()) {
            allLayers.push_back(currentLayer);
        }
    }

    std::cout << "[OvfParser] Parsing complete. Found " << allLayers.size() << " valid layers." << std::endl;
    return allLayers;
}