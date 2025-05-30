#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include "compressor_interface.h"
#include <QObject> //NEW

struct SensorStats {
    size_t streamSize;
    int windowSize;
    double totalCR;
    double totalEncodeThroughputMBs;
    double totalDecodeThroughputMBs;
    double totalEncodeTimeMs;
    double totalDecodeTimeMs;
    double avgEncodeTimePerSampleUs;   // nieuw veld
    double avgDecodeTimePerSampleUs;
    bool valid;
};

class CompressorRunner: public QObject {
    Q_OBJECT
public:
    CompressorRunner() = default;
    void setAlgorithm(const std::string &algorithmName);
    //void runCompression(const std::string& filename, int windowSize, bool useAdaptiveWindowSize);
    void runCompression(const std::string& filename, int windowSize, bool useAdaptiveWindowSize, int updateInterval);
    std::map<std::string, SensorStats> getResults() const;
    std::string getSummaryText() const;
    std::map<std::string, std::vector<double>> getOriginalData() const;
    std::map<std::string, std::vector<double>> getDecompressedData() const;
    const std::map<std::string, std::vector<double>> &getCRPerWindow() const;

private:
    int updateInterval = 1;  // New member variable
    std::string algorithmName;  // <-- keep this private!
    std::map<std::string, SensorStats> results;
    mutable std::mutex resultsMutex;
    void compress_stream(const std::string& sensorName, const std::vector<double>& stream, int startWindowSize, bool useAdaptiveWindowSize);

    void processWindow(const std::vector<double> &currentWindow, std::unique_ptr<CompressorInterface> &compressor, std::vector<double> &allDecoded, bool &firstWindow, double &totalEncodeTimeMs, double &totalDecodeTimeMs, size_t &totalInputBytes, size_t &totalOutputBytes, const std::string& sensorName, bool processWindow);

    std::map<std::string, std::vector<double>> originalData;
    std::map<std::string, std::vector<double>> decompressedData;
    std::map<std::string, std::vector<double>> crPerWindow;
    std::map<std::string, int> windowCounters;
signals:
    void crUpdated(const QString& sensorName, double cr);
    void compressionFinished();
};
