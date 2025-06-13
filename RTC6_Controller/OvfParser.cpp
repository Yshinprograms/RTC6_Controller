#include "OvfParser.h"
#include <fstream>
#include <iostream>
#include <iomanip> // Required for std::hex

// Include the official Protobuf utility for reading delimited messages
#include <google/protobuf/util/delimited_message_util.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

// Include the generated Protobuf headers
#include "open_vector_format.pb.h"
#include "ovf_lut.pb.h"

// Define the maximum laser power for converting Watts to a percentage.
// You can adjust this value to match the maximum power of your laser system.
constexpr double MAX_LASER_POWER_W = 100.0;

std::vector<OvfLayer> OvfParser::parseFile(const std::string& filePath) {
    std::cout << "[OvfParser] Starting to parse file: " << filePath << std::endl;
    std::vector<OvfLayer> allLayers;

    // 1. Open the file as a binary input stream
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "FATAL: [OvfParser] Cannot open file stream." << std::endl;
        return allLayers;
    }

    // 2. Verify the 4-byte OVF Magic Number
    char magic[4];
    file.read(magic, 4);
    if (!file.good() || file.gcount() != 4) {
        std::cerr << "FATAL: [OvfParser] Failed to read magic number." << std::endl;
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
    std::cout << "[OvfParser] JobLUT pointer found at file offset 0x" << std::hex << job_lut_position << std::dec << std::endl;

    // 4. Seek to the JobLUT's position and parse it
    file.seekg(job_lut_position);
    if (!file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to seek to JobLUT position." << std::endl;
        return allLayers;
    }
    google::protobuf::io::IstreamInputStream zero_copy_input_job_lut(&file);
    open_vector_format::JobLUT job_lut;
    if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&job_lut, &zero_copy_input_job_lut, nullptr)) {
        std::cerr << "FATAL: [OvfParser] Failed to parse JobLUT." << std::endl;
        return allLayers;
    }
    std::cout << "[OvfParser] JobLUT parsed successfully." << std::endl;
    std::cout << "  - JobShell Position: 0x" << std::hex << job_lut.jobshellposition() << std::dec << std::endl;
    std::cout << "  - Number of WorkPlane Pointers: " << job_lut.workplanepositions_size() << std::endl;


    // 5. Parse the JobShell (the Job message without the geometry)
    file.seekg(job_lut.jobshellposition());
    if (!file.good()) {
        std::cerr << "FATAL: [OvfParser] Failed to seek to JobShell position." << std::endl;
        return allLayers;
    }
    google::protobuf::io::IstreamInputStream zero_copy_input_shell(&file);
    open_vector_format::Job job_shell;
    if (!google::protobuf::util::ParseDelimitedFromZeroCopyStream(&job_shell, &zero_copy_input_shell, nullptr)) {
        std::cerr << "ERROR: [OvfParser] Failed to parse JobShell. This is the point of failure." << std::endl;
        return allLayers;
    }
    std::cout << "[OvfParser] JobShell parsed successfully. Job Name: " << job_shell.job_meta_data().job_name() << std::endl;


    // 6. Iterate through each WorkPlane, read its data, and convert to your OvfLayer struct
    std::cout << "[OvfParser] Beginning to process " << job_lut.workplanepositions_size() << " work planes..." << std::endl;
    for (int i = 0; i < job_lut.workplanepositions_size(); ++i) {
        // Read the pointer to the WorkPlaneLUT
        file.seekg(job_lut.workplanepositions(i));
        int64_t wp_lut_position;
        file.read(reinterpret_cast<char*>(&wp_lut_position), sizeof(wp_lut_position));

        // Parse the WorkPlaneLUT
        file.seekg(wp_lut_position);
        google::protobuf::io::IstreamInputStream zero_copy_input_wp_lut(&file);
        open_vector_format::WorkPlaneLUT wp_lut;
        google::protobuf::util::ParseDelimitedFromZeroCopyStream(&wp_lut, &zero_copy_input_wp_lut, nullptr);

        // Parse the WorkPlaneShell for this layer
        file.seekg(wp_lut.workplaneshellposition());
        google::protobuf::io::IstreamInputStream zero_copy_input_wp_shell(&file);
        open_vector_format::WorkPlane work_plane_shell;
        google::protobuf::util::ParseDelimitedFromZeroCopyStream(&work_plane_shell, &zero_copy_input_wp_shell, nullptr);

        // --- This is the conversion to your existing data structures ---
        OvfLayer currentLayer;
        currentLayer.z_height_mm = work_plane_shell.z_pos_in_mm();

        // Iterate and parse each VectorBlock for the current WorkPlane
        for (int j = 0; j < wp_lut.vectorblockspositions_size(); ++j) {
            file.seekg(wp_lut.vectorblockspositions(j));
            google::protobuf::io::IstreamInputStream zero_copy_input_vb(&file);
            open_vector_format::VectorBlock vector_block;
            google::protobuf::util::ParseDelimitedFromZeroCopyStream(&vector_block, &zero_copy_input_vb, nullptr);

            // We only convert LineSequences, just like your original parser.
            // We can add more cases (Hatches, Points) here later.
            if (vector_block.has_line_sequence()) {
                OvfPolyline currentPolyline;

                // Get parameters from the JobShell's map
                try {
                    const auto& ovf_params = job_shell.marking_params_map().at(vector_block.marking_params_key());
                    currentPolyline.params.markSpeed_mm_s = ovf_params.laser_speed_in_mm_per_s();
                    currentPolyline.params.focusOffset_mm = ovf_params.laser_focus_shift_in_mm();
                    currentPolyline.params.laserPowerPercent = (ovf_params.laser_power_in_w() / MAX_LASER_POWER_W) * 100.0;
                }
                catch (const std::out_of_range&) {
                    std::cerr << "WARNING: [OvfParser] Marking params key " << vector_block.marking_params_key()
                        << " not found. Using default parameters." << std::endl;
                    // Default parameters will be used from OvfProcessParameters struct
                }

                // Convert points
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