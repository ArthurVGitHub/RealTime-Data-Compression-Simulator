#include "CompressorRunner.h"
#include "drh.h"
#include "delta.h"
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

// Constructor
CompressorRunner::CompressorRunner() {}

void CompressorRunner::setAlgorithm(const std::string &algorithmName) {
    this->algorithmName = algorithmName;
}

void CompressorRunner::compress_stream(const std::string& sensorName, const std::vector<double>& stream, int startWindowSize) {
    if (algorithmName != "DRH" && algorithmName != "Delta") {
        throw std::runtime_error("Compression algorithm not set or unsupported. Call setAlgorithm() first.");
    }

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

            // --- Adaptive window logic ---
            double mean = 0.0;
            for (double v : currentWindow) mean += v;
            mean /= currentWindow.size();
            double var = 0.0;
            for (double v : currentWindow) var += (v - mean) * (v - mean);
            var /= currentWindow.size();

            // Record current window size
            windowSizesUsed.push_back(windowSize);

            // Adjust window size
            if (var < lowVar && windowSize < maxWindow) {
                windowSize++;
            } else if (var > highVar && windowSize > minWindow) {
                windowSize--;
                if (window.size() > windowSize)
                    window.pop_front();
            }
            // --- End adaptive logic ---

            // --- NEW: Use a local compressor instance ---
            std::vector<std::string> encoded;
            std::vector<double> decoded;
            auto startEncode = std::chrono::high_resolution_clock::now();

            if (algorithmName == "DRH") {
                DRH drh;
                encoded = drh.encode(currentWindow);
                auto endEncode = std::chrono::high_resolution_clock::now();

                auto startDecode = std::chrono::high_resolution_clock::now();
                decoded = drh.decode(encoded);
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
            else if (algorithmName == "Delta") {
                Delta delta;
                encoded = delta.encode(currentWindow);
                auto endEncode = std::chrono::high_resolution_clock::now();

                auto startDecode = std::chrono::high_resolution_clock::now();
                decoded = delta.decode(encoded);
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
            // --- END NEW ---
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

void CompressorRunner::runCompression(const std::string& filename, int windowSize) {
    results.clear();
    auto sensorStreams = EMODnetExtractor::extractSensorsData(filename);

    std::vector<std::thread> threads;
    for (const auto& [sensorName, stream] : sensorStreams) {
        if (stream.size() < windowSize || stream.empty()) {
            std::cerr << "Skipping sensor '" << sensorName
                      << "' - stream too small or empty (" << stream.size() << ")\n";
            continue;
        }
        threads.emplace_back(&CompressorRunner::compress_stream, this, sensorName, stream, windowSize);
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
           "Decompression Throughput (MB/s),Compression Time (ms),Decompression Time (ms),Valid\n";
    for (const auto& [sensor, stats] : results) {
        oss << "\"" << sensor << "\","
            << stats.streamSize << ","
            << stats.windowSize << ","
            << std::fixed << std::setprecision(2) << stats.totalCR << ","
            << stats.totalEncodeThroughputMBs << ","
            << stats.totalDecodeThroughputMBs << ","
            << stats.totalEncodeTimeMs << ","
            << stats.totalDecodeTimeMs << ","
            << (stats.valid ? "true" : "false") << "\n";
    }
    return oss.str();
}
