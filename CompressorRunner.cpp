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
        const int minWindow = 4;
        const int maxWindow = 50;
        const double lowVar = 1e-4;
        const double highVar = 1e-2;
        std::deque<double> window;
        std::vector<int> windowSizesUsed;

        for (size_t i = 0; i < stream.size(); ++i) {
            window.push_back(stream[i]);
            if (window.size() > windowSize) window.pop_front();

            if (window.size() == windowSize) {
                std::vector<double> currentWindow(window.begin(), window.end());
                windowSizesUsed.push_back(windowSize);

                // Update window size
                int newWindowSize = WindowOptimizer::updateWindowSize(currentWindow, windowSize,
                                                                      minWindow, maxWindow, lowVar, highVar);
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
    double avgEncodeTimePerSampleUs = (totalEncodeTimeMs * 1000.0) / totalInputBytes;
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
    crPerWindow[sensorName].push_back(cr);


}

/*
void CompressorRunner::compress_stream(const std::string& sensorName, const std::vector<double>& stream, int startWindowSize) {
    // Create compressor ONCE per stream
    auto compressor = createCompressor(algorithmName);

    std::deque<double> window;
    std::vector<double> allDecoded;
    bool firstWindow = true;

    double totalEncodeTimeMs = 0.0;
    double totalDecodeTimeMs = 0.0;
    size_t totalInputBytes = 0;
    size_t totalOutputBytes = 0;

    // Adaptive window parameters
    int windowSize = startWindowSize;
    const int minWindow = 4;
    const int maxWindow = 50;
    const double lowVar = 1e-4;
    const double highVar = 1e-2;

    std::vector<int> windowSizesUsed; // To record window sizes

    for (size_t i = 0; i < stream.size(); ++i) {
        window.push_back(stream[i]);
        if (window.size() > windowSize)
            window.pop_front();

        if (window.size() == windowSize) {
            std::vector<double> currentWindow(window.begin(), window.end());

            // Record current window size
            windowSizesUsed.push_back(windowSize);

            // --- Adaptive window logic using helper ---
            int newWindowSize = WindowOptimizer::updateWindowSize(currentWindow, windowSize, minWindow, maxWindow, lowVar, highVar);
            if (newWindowSize < windowSize && window.size() > newWindowSize)
                window.pop_front();
            windowSize = newWindowSize;

            // --- Compression ---
            std::vector<std::string> encoded;
            std::vector<double> decoded;

            auto startEncode = std::chrono::high_resolution_clock::now();
            encoded = compressor->encode(currentWindow);
            auto endEncode = std::chrono::high_resolution_clock::now();

            auto startDecode = std::chrono::high_resolution_clock::now();
            decoded = compressor->decode(encoded);
            auto endDecode = std::chrono::high_resolution_clock::now();

            double encodeTimeMs = std::chrono::duration<double, std::milli>(endEncode - startEncode).count();
            double decodeTimeMs = std::chrono::duration<double, std::milli>(endDecode - startDecode).count();

            if (firstWindow) {
                allDecoded.insert(allDecoded.end(), decoded.begin(), decoded.end());
                firstWindow = false;
            } else {
                allDecoded.push_back(decoded.back());
            }

            size_t inputBytes = currentWindow.size() * sizeof(double);
            size_t outputBytes = 0;
            for (const auto& s : encoded) outputBytes += s.size();

            totalEncodeTimeMs += encodeTimeMs;
            totalDecodeTimeMs += decodeTimeMs;
            totalInputBytes += inputBytes;
            totalOutputBytes += outputBytes;
        }
    }


    double totalCR = totalOutputBytes == 0 ? 0 : static_cast<double>(totalInputBytes) / totalOutputBytes;
    double totalEncodeThroughputMBs = (totalEncodeTimeMs == 0) ? 0 : (totalInputBytes / (1024.0 * 1024.0)) / (totalEncodeTimeMs / 1000.0);
    double totalDecodeThroughputMBs = (totalDecodeTimeMs == 0) ? 0 : (totalOutputBytes / (1024.0 * 1024.0)) / (totalDecodeTimeMs / 1000.0);

    bool valid = compareVectors(stream, allDecoded);

    // Store results in member variable (thread-safe)
    SensorStats stats = {
            stream.size(),
            startWindowSize,
            totalCR,
            totalEncodeThroughputMBs,
            totalDecodeThroughputMBs,
            totalEncodeTimeMs,
            totalDecodeTimeMs,
            valid
    };
    std::lock_guard<std::mutex> lock(resultsMutex);
    results[sensorName] = stats;
}
*/
void CompressorRunner::runCompression(const std::string& filename, int windowSize, bool useAdaptiveWindowSize) {
    results.clear();
    originalData.clear(); // <--- voeg deze toe als je dat nog niet doet
    decompressedData.clear();
    crPerWindow.clear();

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
}


std::map<std::string, SensorStats> CompressorRunner::getResults() const {
    std::lock_guard<std::mutex> lock(resultsMutex);
    return results;
}

std::string CompressorRunner::getSummaryText() const {
    std::ostringstream oss;
    oss << "Sensor,Stream Size,Window Size,CR,Compression Throughput (MB/s),"
           "Decompression Throughput (MB/s),Compression Time (ms),Decompression Time (ms), Average compression time per Sample, Average decompression time per Sample, Valid\n";
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

