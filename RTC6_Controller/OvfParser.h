// In OvfParser.h
#pragma once

#include <vector>
#include <string>
#include "ProcessData.h"
#include "open_vector_format.pb.h"

class OvfParser {
public:
    OvfParser() = default;

    open_vector_format::Job parseFile(const std::string& filePath);
};