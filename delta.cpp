#include "delta.h"
#include <cstring>

static uint64_t doubleToUint64(double d) {
    uint64_t u;
    std::memcpy(&u, &d, sizeof(double));
    return u;
}

static double uint64ToDouble(uint64_t u) {
    double d;
    std::memcpy(&d, &u, sizeof(double));
    return d;
}

std::vector<int64_t> Delta::encode(const std::vector<double>& data) const {
    std::vector<int64_t> deltas;
    if (data.empty()) return deltas;

    uint64_t prev = doubleToUint64(data[0]);
    deltas.push_back(static_cast<int64_t>(prev));

    for (size_t i = 1; i < data.size(); ++i) {
        uint64_t curr = doubleToUint64(data[i]);
        deltas.push_back(static_cast<int64_t>(curr) - static_cast<int64_t>(prev));
        prev = curr;
    }
    return deltas;
}

std::vector<double> Delta::decode(const std::vector<int64_t>& deltas) const {
    std::vector<double> data;
    if (deltas.empty()) return data;

    int64_t curr = deltas[0];
    data.push_back(uint64ToDouble(static_cast<uint64_t>(curr)));

    for (size_t i = 1; i < deltas.size(); ++i) {
        curr += deltas[i];
        data.push_back(uint64ToDouble(static_cast<uint64_t>(curr)));
    }
    return data;
}
