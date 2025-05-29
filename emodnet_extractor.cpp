#include "emodnet_extractor.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include <cctype>

namespace {
    // Helper: Split a line by delimiter (tab)
    std::vector<std::string> splitLine(const std::string& line, char delim = '\t') {
        std::vector<std::string> cols;
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, delim)) cols.push_back(item);
        return cols;
    }

    // Helper: Check if a string is numeric
    bool isNumeric(const std::string& s) {
        char* end = nullptr;
        std::strtod(s.c_str(), &end);
        return end != s.c_str() && *end == '\0';
    }

    // Helper: Decide if a column is metadata or QV
    bool is_metadata_column(const std::string& colname) {
        static const std::set<std::string> meta_keywords = {
                "cruise", "station", "type", "date", "time", "longitude", "latitude",
                "local_cdi_id", "edmo_code", "bot. depth"
        };
        std::string lower = colname;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        // skip QV columns
        if (lower.find("qv:") == 0) return true;
        // skip metadata
        for (const auto& kw : meta_keywords)
            if (lower.find(kw) != std::string::npos)
                return true;
        return false;
    }
}

std::map<std::string, std::vector<double>> EMODnetExtractor::extractSensorsData(const std::string& filename) {
    std::map<std::string, std::vector<double>> sensors;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return {};
    }
    std::string line;

    // 1. Skip all comment/empty lines until header
    while (std::getline(file, line)) {
        std::string trimmed = line;
        trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        if (!trimmed.empty() && trimmed[0] != '/') break;
    }
    if (line.empty()) {
        std::cerr << "File is empty or unreadable." << std::endl;
        return {};
    }

    // 2. Parse header
    std::vector<std::string> headers = splitLine(line, '\t');
    size_t nCols = headers.size();

    // 3. Find relevant (non-metadata, non-QV) columns
    std::vector<size_t> sensorCols;
    std::vector<std::string> sensorNames;
    for (size_t i = 0; i < headers.size(); ++i) {
        if (!is_metadata_column(headers[i])) {
            sensorCols.push_back(i);
            sensorNames.push_back(headers[i]);
            sensors[headers[i]] = {};
        }
    }

    // 4. Parse data rows
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '/') continue;
        auto cols = splitLine(line, '\t');
        for (size_t j = 0; j < sensorCols.size(); ++j) {
            size_t idx = sensorCols[j];
            if (idx < cols.size()) {
                std::string val = cols[idx];
                if (isNumeric(val)) {
                    sensors[sensorNames[j]].push_back(std::stod(val));
                }
            }
        }
    }
    // In EMODnetExtractor::extractSensorsData, after parsing data:
    for (const auto& [name, vec] : sensors) {
        std::cout << "Sensor: " << name << " (first 5 values): ";
        for (size_t i = 0; i < std::min<size_t>(5, vec.size()); ++i)
            std::cout << vec[i] << " ";
        std::cout << "\n";
    }


    return sensors;
}