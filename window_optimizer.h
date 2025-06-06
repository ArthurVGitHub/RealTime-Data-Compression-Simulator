//
// Created by arthu on 18/05/2025.
//

#ifndef THESIS_WINDOW_OPTIMIZER_H
#define THESIS_WINDOW_OPTIMIZER_H
#pragma once
#include <vector>
#include "compressor_interface.h"

class WindowOptimizer {
public:
    static int updateWindowSize(const std::vector<double>& window, int currentSize, int minWindow, int maxWindow, double lowVar, double highVar);
    //static int optimizeWindowSizeForCR(const std::vector<double>& pastWindow, int currentSize, int minWindow, int maxWindow, CompressorInterface* compressor);
    static int updateWindowSizeCV(const std::vector<double> &window, int currentSize, int minWindow, int maxWindow, double lowVar, double highVar);

    static int updateWindowSizeSLOPE(const std::vector<double> &window, int currentSize, int minWindow, int maxWindow, double lowVar, double highVar, double maxSlopeVar);
};

#endif //THESIS_WINDOW_OPTIMIZER_H