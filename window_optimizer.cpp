//
// Created by arthu on 18/05/2025.
//

#include <cstdint>
#include <iostream>
#include <numeric>
#include "window_optimizer.h"
int WindowOptimizer::updateWindowSize(
        const std::vector<double> &window, int currentSize, int minWindow, int maxWindow,
        double lowVar, double highVar)
{
    if (window.empty()) return currentSize;
    double mean = 0.0;
    for (double v : window) mean += v;
    mean /= window.size();
    double var = 0.0;
    for (double v : window) var += (v - mean) * (v - mean);
    var /= window.size();
    int newSize = currentSize;
    if (var < lowVar && currentSize < maxWindow) newSize = currentSize + 1;
    else if (var > highVar && currentSize > minWindow) newSize = currentSize - 1;
    //std::cout << "Window MIN: " << minWindow << " | New size: " << newSize << "\n";
    //std::cout << "Window variance: " << var << " | New size: " << newSize << "\n";
    return newSize;
}

int WindowOptimizer::updateWindowSizeCV(
        const std::vector<double> &window, int currentSize, int minWindow, int maxWindow,
        double lowVar, double highVar)
{
    if (window.empty()) return currentSize;
    double mean = 0.0;
    for (double v : window) mean += v;
    mean /= window.size();

    double var = 0.0;
    for (double v : window) var += (v - mean) * (v - mean);
    var /= window.size();

    double normVar = 0.0;
    if (mean != 0.0) {
        normVar = var / (mean * mean);  // "Normalized variant
    } else {
        normVar = var;  // Fallback if mean=0
    }

    int newSize = currentSize;
    if (normVar < lowVar && currentSize < maxWindow)
        newSize = currentSize + 1;
    else if (normVar > highVar && currentSize > minWindow)
        newSize = currentSize - 1;

    //std::cout << "Window MIN: " << minWindow << " | New size: " << newSize << "\n";
    return newSize;

}

int WindowOptimizer::updateWindowSizeSLOPE(
        const std::vector<double>& window, int currentSize, int minWindow, int maxWindow,
        double lowVar, double highVar, double maxSlopeVar = 0.1) // New parameter
{
    if (window.empty()) return currentSize;

    // 1. Compute normalized variance (as before)
    double mean = 0.0;
    for (double v : window) mean += v;
    mean /= window.size();
    double var = 0.0;
    for (double v : window) var += (v - mean) * (v - mean);
    var /= window.size();
    double normVar = (mean != 0.0) ? var / (mean * mean) : var;

    // 2. Compute trend slope variance (new logic)
    std::vector<double> diffs;
    for (size_t i = 1; i < window.size(); ++i) {
        diffs.push_back(window[i] - window[i-1]);
    }
    double slopeVar = 0.0;
    if (!diffs.empty()) {
        double avgDiff = std::accumulate(diffs.begin(), diffs.end(), 0.0) / diffs.size();
        for (double d : diffs) slopeVar += (d - avgDiff) * (d - avgDiff);
        slopeVar /= diffs.size();
    }

    // 3. Adjust window size based on BOTH normalized variance and slope variance
    int newSize = currentSize;
    if (slopeVar < maxSlopeVar) {  // Steady trend (e.g., linear increase)
        // Allow larger windows even if variance is high
        if (currentSize < maxWindow) newSize = currentSize + 1;
    } else {  // Noisy or non-linear data
        if (normVar < lowVar && currentSize < maxWindow) newSize = currentSize + 1;
        else if (normVar > highVar && currentSize > minWindow) newSize = currentSize - 1;
    }
    //std::cout << "Slope variance: " << slopeVar << " | Norm variance: " << normVar << "\n";

    return newSize;
}




//UNDER TEST:
/*
int WindowOptimizer::optimizeWindowSizeForCR(const std::vector<double> &pastWindow, int currentSize, int minWindow,
                                             int maxWindow, CompressorInterface *compressor) {
    const int searchRadius = 2; // check ±2 around current size
    int bestWindowSize = currentSize;
    double bestCR = 0.0;

    for (int delta = -searchRadius; delta <= searchRadius; ++delta) {
        int testSize = currentSize + delta;
        if (testSize < minWindow || testSize > maxWindow) continue;
        if (pastWindow.size() < testSize) continue;

        std::vector<double> testWindow(
                pastWindow.end() - testSize, pastWindow.end()
        );

        std::vector<uint8_t> compressed = compressor->encode(testWindow);
        std::vector<double> decompressed = compressor->decode(compressed);

        if (decompressed != testWindow) continue; // Skip if not lossless

        size_t inputBytes = testWindow.size() * sizeof(double);
        size_t outputBytes = compressed.size();
        if (outputBytes == 0) continue;

        double cr = static_cast<double>(inputBytes) / outputBytes;
        if (cr > bestCR) {
            bestCR = cr;
            bestWindowSize = testSize;
        }
    }

    return bestWindowSize;
}
*/