#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>

struct SensorStats {
    size_t streamSize;
    int windowSize;
    double totalCR;
    double totalEncodeThroughputMBs;
    double totalDecodeThroughputMBs;
    double totalEncodeTimeMs;
    double totalDecodeTimeMs;
    bool valid;
};

class CompressorRunner {
public:
    CompressorRunner();
    void setAlgorithm(const std::string &algorithmName);
    void runCompression(const std::string& filename, int windowSize);
    std::map<std::string, SensorStats> getResults() const;
    std::string getSummaryText() const;

private:
    std::string algorithmName;  // <-- keep this private!
    std::map<std::string, SensorStats> results;
    mutable std::mutex resultsMutex;
    void compress_stream(const std::string& sensorName, const std::vector<double>& stream, int startWindowSize);
};
