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

/*namespace {
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
}*/

/*std::map<std::string, std::vector<double>> EMODnetExtractor::extractSensorsData(const std::string& filename) {
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

    for (const auto& [name, vec] : sensors) {
        std::cout << "Sensor: " << name << " (first 5 values): ";
        for (size_t i = 0; i < std::min<size_t>(5, vec.size()); ++i)
            std::cout << vec[i] << " ";
        std::cout << "\n";
    }


    return sensors;
}*/
namespace {
    // Helper: Split a line by delimiter
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

    // Helper: Detect delimiter
    char detectDelimiter(const std::string& firstLine) {
        size_t tabCount = std::count(firstLine.begin(), firstLine.end(), '\t');
        size_t commaCount = std::count(firstLine.begin(), firstLine.end(), ',');
        return (tabCount > commaCount) ? '\t' : ',';
    }

    // Helper: Trim whitespace from both ends
    std::string trim(const std::string& s) {
        auto start = s.begin();
        while (start != s.end() && std::isspace(static_cast<unsigned char>(*start))) ++start;
        auto end = s.end();
        do { --end; } while (std::distance(start, end) > 0 && std::isspace(static_cast<unsigned char>(*end)));
        return std::string(start, end + 1);
    }

    // Helper: Lowercase
    std::string toLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(), ::tolower);
        return out;
    }

    // Helper: Decide if a column is metadata or QV
    bool is_metadata_column(const std::string& colname, bool is_csv) {
        static const std::set<std::string> txt_meta_keywords = {
                "cruise", "station", "type", "date", "time", "longitude", "latitude",
                "local_cdi_id", "edmo_code", "bot. depth"
        };
        static const std::set<std::string> csv_meta_keywords = {
                "cruise", "station", "type", "date", "time", "longitude", "latitude",
                "local_cdi_id", "edmo_code", "bot. depth", "qv:seadatanet"
        };

        std::string lower = toLower(trim(colname));
        if (lower.find("qv:") == 0) return true;

        if (is_csv) {
            if (lower.find("%time") == 0 ||
                lower.find("field.header.") == 0 ||
                lower.find("field.hour") == 0) {
                return true;
            }
            return csv_meta_keywords.count(lower);
        } else {
            for (const auto& kw : txt_meta_keywords)
                if (lower.find(kw) != std::string::npos)
                    return true;
            return false;
        }
    }
}


std::map<std::string, std::vector<double>> EMODnetExtractor::extractSensorsData(const std::string& filename) {
    // For CSV, specify which sensors you want (normalized)
    static const std::set<std::string> sensorsToKeepRaw = {
            "conductivity", "depth", "local_density", "pressure",
            "salinity", "sound_velocity", "specific_conductivity", "temperature"
    };
    std::set<std::string> sensorsToKeep;
    for (const auto& s : sensorsToKeepRaw) sensorsToKeep.insert(toLower(trim(s)));

    std::map<std::string, std::vector<double>> sensors;
    std::ifstream file(filename);
    if (!file.is_open()) return {};

    // Read first line to detect delimiter and headers
    std::string firstLine;
    while (std::getline(file, firstLine)) {
        if (!firstLine.empty() && firstLine[0] != '/') break;
    }
    if (firstLine.empty()) {
        std::cerr << "No data lines found in file." << std::endl;
        return {};
    }
    char delim = detectDelimiter(firstLine);

    // Parse headers
    std::vector<std::string> headers = splitLine(firstLine, delim);
    if (headers.empty()) {
        std::cerr << "No headers found in file." << std::endl;
        return {};
    }

    // Detect if CSV (any header starts with "field.")
    bool is_csv = std::any_of(headers.begin(), headers.end(),
                              [](const std::string& h) { return h.find("field.") == 0; });

    // Find sensor columns
    std::vector<size_t> sensorCols;
    std::vector<std::string> sensorNames;
    for (size_t i = 0; i < headers.size(); ++i) {
        std::string header = headers[i];
        if (is_csv) {
            std::string cleanName = toLower(trim(header.substr(header.find("field.") == 0 ? 6 : 0)));
            if (!is_metadata_column(header, true) && sensorsToKeep.count(cleanName)) {
                sensorCols.push_back(i);
                sensorNames.push_back(cleanName);
                std::cout << "Extracting CSV sensor: '" << cleanName << "' (original: '" << header << "')\n";
            }
        } else {
            if (!is_metadata_column(header, false)) {
                sensorCols.push_back(i);
                sensorNames.push_back(toLower(trim(header)));
                std::cout << "Extracting TXT sensor: '" << header << "'\n";
            }
        }
    }

    if (sensorCols.empty()) {
        std::cerr << "No matching sensor columns found in file." << std::endl;
        return {};
    }

    // Parse data rows
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '/') continue;
        auto cols = splitLine(line, delim);
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

    for (const auto& [name, vec] : sensors) {
        std::cout << "Sensor: " << name << " (first 5 values): ";
        for (size_t i = 0; i < std::min<size_t>(5, vec.size()); ++i)
            std::cout << vec[i] << " ";
        std::cout << "\n";
    }

    return sensors;
}