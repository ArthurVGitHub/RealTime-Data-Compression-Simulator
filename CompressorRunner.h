#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include "compressor_interface.h"

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
    CompressorRunner() = default;
    void setAlgorithm(const std::string &algorithmName);
    void runCompression(const std::string& filename, int windowSize, bool useAdaptiveWindowSize);
    std::map<std::string, SensorStats> getResults() const;
    std::string getSummaryText() const;
    std::map<std::string, std::vector<double>> getOriginalData() const;
    std::map<std::string, std::vector<double>> getDecompressedData() const;
private:
    std::string algorithmName;  // <-- keep this private!
    std::map<std::string, SensorStats> results;
    mutable std::mutex resultsMutex;
    void compress_stream(const std::string& sensorName, const std::vector<double>& stream, int startWindowSize, bool useAdaptiveWindowSize);

    void processWindow(const std::vector<double> &currentWindow, std::unique_ptr<CompressorInterface> &compressor, std::vector<double> &allDecoded, bool &firstWindow, double &totalEncodeTimeMs, double &totalDecodeTimeMs, size_t &totalInputBytes, size_t &totalOutputBytes);
    std::map<std::string, std::vector<double>> originalData;
    std::map<std::string, std::vector<double>> decompressedData;
};
