//
// Created by arthu on 18/05/2025.
//

#ifndef THESIS_WINDOW_OPTIMIZER_H
#define THESIS_WINDOW_OPTIMIZER_H
#pragma once
#include <vector>

class WindowOptimizer {
public:
    static int updateWindowSize(const std::vector<double>& window, int currentSize, int minWindow, int maxWindow, double lowVar, double highVar);
};

#endif //THESIS_WINDOW_OPTIMIZER_H
