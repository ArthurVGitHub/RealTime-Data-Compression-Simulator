#include <vector>
#include <cmath>

bool compareVectors(const std::vector<double>& a, const std::vector<double>& b, double epsilon = 1e-9) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::abs(a[i] - b[i]) > epsilon) return false;
    }
    return true;
}
