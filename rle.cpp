#include "rle.h"
#include <cstring>

// --------- INT64_T VERSION (for DeltaRle) ---------

std::string Rle::encode(const std::vector<int64_t>& data) const {
    std::string out;
    if (data.empty()) return out;

    int64_t current = data[0];
    uint32_t count = 1;
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] == current && count < UINT32_MAX) {
            ++count;
        } else {
            out.append(reinterpret_cast<const char*>(&current), sizeof(int64_t));
            out.append(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
            current = data[i];
            count = 1;
        }
    }
    out.append(reinterpret_cast<const char*>(&current), sizeof(int64_t));
    out.append(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
    return out;
}

std::vector<int64_t> Rle::decode(const std::string& encoded) const {
    std::vector<int64_t> out;
    size_t pos = 0;
    while (pos + sizeof(int64_t) + sizeof(uint32_t) <= encoded.size()) {
        int64_t value;
        uint32_t count;
        std::memcpy(&value, encoded.data() + pos, sizeof(int64_t));
        pos += sizeof(int64_t);
        std::memcpy(&count, encoded.data() + pos, sizeof(uint32_t));
        pos += sizeof(uint32_t);
        out.insert(out.end(), count, value);
    }
    return out;
}

// --------- DOUBLE VERSION (for general use) ---------

std::vector<std::string> Rle::encode(const std::vector<double>& data) {
    std::string out;
    if (data.empty()) return {out};

    double current = data[0];
    uint32_t count = 1;
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] == current && count < UINT32_MAX) {
            ++count;
        } else {
            out.append(reinterpret_cast<const char*>(&current), sizeof(double));
            out.append(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
            current = data[i];
            count = 1;
        }
    }
    out.append(reinterpret_cast<const char*>(&current), sizeof(double));
    out.append(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
    return {out};
}

std::vector<double> Rle::decode(const std::vector<std::string>& encoded) {
    if (encoded.empty()) return {};
    const std::string& enc = encoded[0];
    std::vector<double> out;
    size_t pos = 0;
    while (pos + sizeof(double) + sizeof(uint32_t) <= enc.size()) {
        double value;
        uint32_t count;
        std::memcpy(&value, enc.data() + pos, sizeof(double));
        pos += sizeof(double);
        std::memcpy(&count, enc.data() + pos, sizeof(uint32_t));
        pos += sizeof(uint32_t);
        out.insert(out.end(), count, value);
    }
    return out;
}
