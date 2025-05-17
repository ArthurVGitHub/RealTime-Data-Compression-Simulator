#pragma once
#include <vector>
#include <string>
#include <map>

class EMODnetExtractor {
public:
    static std::map<std::string, std::vector<double>> extractSensorsData(const std::string& filename);
};
