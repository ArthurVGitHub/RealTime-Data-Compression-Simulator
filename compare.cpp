#include <vector>
#include <cmath>
#include "compare.h"
bool compareVectors(const std::vector<double>& a, const std::vector<double>& b, double epsilon) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::abs(a[i] - b[i]) > epsilon) return false;
    }
    return true;
}
