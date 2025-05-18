//
// Created by arthu on 17/05/2025.
//

#include "delta.h"
#include <cstring>

uint64_t Delta::doubleToUint64(double d) const {
    uint64_t u;
    std::memcpy(&u, &d, sizeof(double));
    return u;
}

double Delta::uint64ToDouble(uint64_t u) const {
    double d;
    std::memcpy(&d, &u, sizeof(double));
    return d;
}

std::vector<int64_t> Delta::deltaEncodeLossless(const std::vector<double>& data) const {
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

std::vector<double> Delta::deltaDecodeLossless(const std::vector<int64_t>& deltas) const {
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

std::vector<std::string> Delta::encode(const std::vector<double>& data) {
    std::vector<int64_t> deltas = deltaEncodeLossless(data);

    // Serialize deltas into a byte stream
    std::string serialized;
    for (int64_t d : deltas) {
        serialized.append(reinterpret_cast<const char*>(&d), sizeof(int64_t));
    }

    return {serialized};
}

std::vector<double> Delta::decode(const std::vector<std::string>& encodedValues) {
    std::vector<int64_t> deltas;
    if (encodedValues.empty()) return {};

    const std::string& data = encodedValues[0];
    size_t count = data.size() / sizeof(int64_t);
    deltas.resize(count);
    std::memcpy(deltas.data(), data.data(), count * sizeof(int64_t));

    return deltaDecodeLossless(deltas);
}
