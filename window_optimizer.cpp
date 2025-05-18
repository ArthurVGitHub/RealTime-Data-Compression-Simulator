//
// Created by arthu on 18/05/2025.
//

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
