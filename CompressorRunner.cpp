#include "CompressorRunner.h"
#include "compressor_factory.h"
#include "window_optimizer.h"
#include "compare.h"
#include "emodnet_extractor.h"
#include <vector>
#include <deque>
#include <chrono>
#include <iomanip>
#include <thread>
#include <fstream>
#include <sstream>
#include <memory>
#include <iostream>


void CompressorRunner::setAlgorithm(const std::string &algorithmName) {
    this->algorithmName = algorithmName;
}

void CompressorRunner::compress_stream(const std::string& sensorName, const std::vector<double>& stream, int windowSize, bool adaptiveWindowSize) {
    auto compressor = createCompressor(algorithmName);
    std::vector<double> allDecoded;
    bool firstWindow = true;

    double totalEncodeTimeMs = 0.0;
    double totalDecodeTimeMs = 0.0;
    size_t totalInputBytes = 0;
    size_t totalOutputBytes = 0;

    if (adaptiveWindowSize) {
        // Adaptive window logic
        const int minWindow = 2;
        const int maxWindow = 60;
        const double lowVar = 1e-4; // Below this signal is stable, increase window size
        const double highVar = 1e-2; // WAS 1e-2
        const double maxSlopeVar = 1e-2;
        std::deque<double> window;
        std::vector<int> windowSizesUsed;

        for (size_t i = 0; i < stream.size(); ++i) {
            window.push_back(stream[i]);
            if (window.size() > windowSize) window.pop_front();

            if (window.size() == windowSize) {
                std::vector<double> currentWindow(window.begin(), window.end());
                windowSizesUsed.push_back(windowSize);

                // Update window size
                int newWindowSize = WindowOptimizer::updateWindowSizeSLOPE(currentWindow, windowSize,
                                                                      minWindow, maxWindow, lowVar, highVar, maxSlopeVar); //add maxSlopeVar for SLOPE
                if (newWindowSize < windowSize && window.size() > newWindowSize) window.pop_front();
                windowSize = newWindowSize;

                // Process window
                processWindow(currentWindow, compressor, allDecoded, firstWindow,
                              totalEncodeTimeMs, totalDecodeTimeMs, totalInputBytes, totalOutputBytes, sensorName, adaptiveWindowSize);
            }
        }
    } else {
        // Fixed window logic
        for (size_t i = 0; i < stream.size(); i += windowSize) {
            auto start = stream.begin() + i;
            auto end = (i + windowSize <= stream.size()) ? start + windowSize : stream.end();
            std::vector<double> currentWindow(start, end);

            processWindow(currentWindow, compressor, allDecoded, firstWindow,
                          totalEncodeTimeMs, totalDecodeTimeMs, totalInputBytes, totalOutputBytes, sensorName, adaptiveWindowSize);;
        }
    }

    // Calculate stats and store results (same for both cases)
    double totalCR = totalOutputBytes == 0 ? 0 : static_cast<double>(totalInputBytes) / totalOutputBytes;
    double totalEncodeThroughputMBs = (totalEncodeTimeMs == 0) ? 0 : (totalInputBytes / (1024.0 * 1024.0)) / (totalEncodeTimeMs / 1000.0);
    double totalDecodeThroughputMBs = (totalDecodeTimeMs == 0) ? 0 : (totalOutputBytes / (1024.0 * 1024.0)) / (totalDecodeTimeMs / 1000.0);
    double avgEncodeTimePerSampleUs = (totalEncodeTimeMs * 1000.0) / totalInputBytes; // *1000 → microseconds (µs).
    double avgDecodeTimePerSampleUs = (totalDecodeTimeMs * 1000.0) / totalOutputBytes;

    bool valid = compareVectors(stream, allDecoded);

    // Store results in member variable (thread-safe)
    SensorStats stats = {
            stream.size(),
            windowSize,
            totalCR,
            totalEncodeThroughputMBs,
            totalDecodeThroughputMBs,
            totalEncodeTimeMs,
            totalDecodeTimeMs,
            avgEncodeTimePerSampleUs,
            avgDecodeTimePerSampleUs,
            valid
    };

    {
        std::lock_guard lock(resultsMutex);
        results[sensorName] = stats;
        originalData[sensorName] = stream;
        decompressedData[sensorName] = allDecoded;
    }


    std::lock_guard<std::mutex> lock(resultsMutex);
    results[sensorName] = stats;
}

void CompressorRunner::processWindow(const std::vector<double>& currentWindow,
                                     std::unique_ptr<CompressorInterface>& compressor,
                                     std::vector<double>& allDecoded, bool& firstWindow,
                                     double& totalEncodeTimeMs, double& totalDecodeTimeMs,
                                     size_t& totalInputBytes, size_t& totalOutputBytes, const std::string& sensorName, bool adaptiveWindowSize) {
    std::vector<std::string> encoded;
    std::vector<double> decoded;

    // Encode
    auto startEncode = std::chrono::high_resolution_clock::now();
    encoded = compressor->encode(currentWindow);
    auto endEncode = std::chrono::high_resolution_clock::now();

    // Decode
    auto startDecode = std::chrono::high_resolution_clock::now();
    decoded = compressor->decode(encoded);
    auto endDecode = std::chrono::high_resolution_clock::now();

    // Update decoded data
    if (adaptiveWindowSize) {
        if (firstWindow) {
            allDecoded.insert(allDecoded.end(), decoded.begin(), decoded.end());
            firstWindow = false;
        } else {
            allDecoded.push_back(decoded.back());
        }
    }else {
        // Fixed window logic
        allDecoded.insert(allDecoded.end(), decoded.begin(), decoded.end());
    }

    // Update metrics
    size_t inputBytes = currentWindow.size() * sizeof(double);
    size_t outputBytes = 0;
    for (const auto& s : encoded) outputBytes += s.size();

    totalEncodeTimeMs += std::chrono::duration<double, std::milli>(endEncode - startEncode).count();
    totalDecodeTimeMs += std::chrono::duration<double, std::milli>(endDecode - startDecode).count();
    totalInputBytes += inputBytes;
    totalOutputBytes += outputBytes;
    double cr = (double)inputBytes / outputBytes;

    windowCounters[sensorName]++;
    if (windowCounters[sensorName] % updateInterval == 0) {
        emit crUpdated(QString::fromStdString(sensorName), cr);
        windowCounters[sensorName] = 0;  // Reset counter
    }
    crPerWindow[sensorName].push_back(cr);
}

/*void CompressorRunner::runCompression(const std::string& filename, int windowSize, bool useAdaptiveWindowSize, int updateInterval) {
    results.clear();
    originalData.clear();
    decompressedData.clear();
    crPerWindow.clear();
    windowCounters.clear();
    this->updateInterval = updateInterval;
    auto sensorStreams = EMODnetExtractor::extractSensorsData(filename);

    std::vector<std::thread> threads;
    for (const auto& [sensorName, stream] : sensorStreams) {
        originalData[sensorName] = stream; // <-- altijd opslaan!
        if (stream.size() < windowSize || stream.empty()) {
            std::cerr << "Skipping sensor '" << sensorName
                      << "' - stream too small or empty (" << stream.size() << ")\n";
            continue;
        }
        threads.emplace_back(&CompressorRunner::compress_stream, this, sensorName, stream, windowSize, useAdaptiveWindowSize);
    }
    for (auto& t : threads) t.join();
    emit compressionFinished();
}*/

void CompressorRunner::runCompression(const std::string& filename, int windowSize, bool useAdaptiveWindowSize, int updateInterval) {
    results.clear();
    originalData.clear();
    decompressedData.clear();
    crPerWindow.clear();
    windowCounters.clear();
    dataCharacteristics.clear();
    this->updateInterval = updateInterval;

    auto sensorStreams = EMODnetExtractor::extractSensorsData(filename);

    // 1. Compute data characteristics for all sensors
    for (const auto& [sensorName, stream] : sensorStreams) {
        DataCharacteristics stats;
        stats.sensorName = sensorName;
        stats.unit = ""; // Set this if you have units
        stats.streamSizeBytes = stream.size() * sizeof(double);
        stats.numSamples = stream.size();

        if (!stream.empty()) {
            double sum = 0.0;
            stats.minValue = stream[0];
            stats.maxValue = stream[0];
            for (double v : stream) {
                sum += v;
                if (v < stats.minValue) stats.minValue = v;
                if (v > stats.maxValue) stats.maxValue = v;
            }
            stats.avgValue = sum / stream.size();

            double variance = 0.0;
            for (double v : stream) {
                variance += (v - stats.avgValue) * (v - stats.avgValue);
            }
            stats.stdDev = std::sqrt(variance / stream.size());
            stats.cv = (stats.avgValue != 0) ? (stats.stdDev / stats.avgValue) : 0.0;
        }
        dataCharacteristics[sensorName] = stats;
        originalData[sensorName] = stream; // Also store original data
    }

    // 2. Start compression threads for valid streams
    std::vector<std::thread> threads;
    for (const auto& [sensorName, stream] : sensorStreams) {
        if (stream.size() < windowSize || stream.empty()) {
            std::cerr << "Skipping sensor '" << sensorName
                      << "' - stream too small or empty (" << stream.size() << ")\n";
            continue;
        }
        threads.emplace_back(&CompressorRunner::compress_stream, this, sensorName, stream, windowSize, useAdaptiveWindowSize);
    }
    for (auto& t : threads) t.join();
    emit compressionFinished();
}


std::map<std::string, SensorStats> CompressorRunner::getResults() const {
    std::lock_guard<std::mutex> lock(resultsMutex);
    return results;
}

std::string CompressorRunner::getSummaryText() const {
    std::ostringstream oss;
    oss << "Sensor,Stream Size,Window Size,CR,Compression Throughput (MB/s),"
           "Decompression Throughput (MB/s),Compression Time (ms),Decompression Time (ms), Avg. comp. time (µs/sample), Avg. decomp. time (µs/sample), Valid\n";
    for (const auto& [sensor, stats] : results) {
        oss << "\"" << sensor << "\","
            << stats.streamSize << ","
            << stats.windowSize << ","
            << std::fixed << std::setprecision(2) << stats.totalCR << ","
            << stats.totalEncodeThroughputMBs << ","
            << stats.totalDecodeThroughputMBs << ","
            << stats.totalEncodeTimeMs << ","
            << stats.totalDecodeTimeMs << ","
            << stats.avgEncodeTimePerSampleUs << ","
            << stats.avgDecodeTimePerSampleUs << ","
            << (stats.valid ? "true" : "false") << "\n";
    }
    return oss.str();
}

std::string CompressorRunner::getDataCharacteristicsText() const {
    std::ostringstream oss;
    oss << "Sensor (Unit),Stream Size (B),Samples,Avg,Std Dev,CV,Min,Max\n";
    for (const auto& [sensor, stats] : dataCharacteristics) {
        double cv = (stats.avgValue != 0) ? (stats.stdDev / stats.avgValue) : 0.0;
        oss << "\"" << stats.sensorName << " (" << stats.unit << ")\","
            << stats.streamSizeBytes << ","
            << stats.numSamples << ","
            << stats.avgValue << ","
            << stats.stdDev << ","
            << cv << ","
            << stats.minValue << ","
            << stats.maxValue << "\n";
    }
    return oss.str();
}


std::map<std::string, std::vector<double>> CompressorRunner::getOriginalData() const {
    std::lock_guard lock(resultsMutex);
    return originalData;
}

std::map<std::string, std::vector<double>> CompressorRunner::getDecompressedData() const {
    std::lock_guard lock(resultsMutex);
    return decompressedData;
}

const std::map<std::string, std::vector<double>>& CompressorRunner::getCRPerWindow() const {
    return crPerWindow;
}