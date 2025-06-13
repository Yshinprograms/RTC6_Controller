#pragma once

#include <vector>
#include <string>
#include "ProcessData.h"

class OvfParser {
public:
    OvfParser() = default;

    // The public method remains the same for now to ensure the rest of your
    // application, which expects this data structure, continues to function.
    std::vector<OvfLayer> parseFile(const std::string& filePath);
};