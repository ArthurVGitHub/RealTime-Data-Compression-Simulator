//
// Created by arthu on 18/05/2025.
//

#include <cstdint>
#include "window_optimizer.h"

int WindowOptimizer::updateWindowSize(const std::vector<double> &window, int currentSize, int minWindow, int maxWindow,
                                      double lowVar, double highVar) {
    if (window.empty()) return currentSize;
    double mean = 0.0;
    for (double v : window) mean += v;
    mean /= window.size();
    double var = 0.0;
    for (double v : window) var += (v - mean) * (v - mean);
    var /= window.size();

    if (var < lowVar && currentSize < maxWindow) return currentSize + 1;
    if (var > highVar && currentSize > minWindow) return currentSize - 1;
    return currentSize;
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
