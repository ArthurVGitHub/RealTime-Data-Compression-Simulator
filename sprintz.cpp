//
// Created by arthu on 22/05/2025.
//

#include "sprintz.h"
#include <sstream>
#include <cmath>
#include <cstring>

// Quantize doubles to int16_t
std::vector<int16_t> SprintzCompressor::quantize(const std::vector<double>& data, double scale) const {
    std::vector<int16_t> out(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        out[i] = static_cast<int16_t>(std::round(data[i] * scale));
    return out;
}

std::vector<double> SprintzCompressor::dequantize(const std::vector<int16_t>& data, double scale) const {
    std::vector<double> out(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        out[i] = static_cast<double>(data[i]) / scale;
    return out;
}

// Delta encode/decode
std::vector<int16_t> SprintzCompressor::delta_encode(const std::vector<int16_t>& data) const {
    std::vector<int16_t> deltas(data.size());
    if (data.empty()) return deltas;
    deltas[0] = data[0];
    for (size_t i = 1; i < data.size(); ++i)
        deltas[i] = data[i] - data[i-1];
    return deltas;
}

std::vector<int16_t> SprintzCompressor::delta_decode(const std::vector<int16_t>& deltas) const {
    std::vector<int16_t> data(deltas.size());
    if (deltas.empty()) return data;
    data[0] = deltas[0];
    for (size_t i = 1; i < deltas.size(); ++i)
        data[i] = data[i-1] + deltas[i];
    return data;
}

// RLE encode/decode
std::vector<std::pair<int16_t, int>> SprintzCompressor::rle_encode(const std::vector<int16_t>& deltas) const {
    std::vector<std::pair<int16_t, int>> rle;
    if (deltas.empty()) return rle;
    int16_t prev = deltas[0];
    int count = 1;
    for (size_t i = 1; i < deltas.size(); ++i) {
        if (deltas[i] == prev) {
            ++count;
        } else {
            rle.emplace_back(prev, count);
            prev = deltas[i];
            count = 1;
        }
    }
    rle.emplace_back(prev, count);
    return rle;
}

std::vector<int16_t> SprintzCompressor::rle_decode(const std::vector<std::pair<int16_t, int>>& rle) const {
    std::vector<int16_t> deltas;
    for (const auto& p : rle) {
        for (int i = 0; i < p.second; ++i)
            deltas.push_back(p.first);
    }
    return deltas;
}

// Serialize RLE pairs to string (binary, little-endian)
std::string SprintzCompressor::serialize(const std::vector<std::pair<int16_t, int>>& rle) const {
    std::string out;
    out.reserve(rle.size() * (sizeof(int16_t) + sizeof(int)));
    for (const auto& p : rle) {
        out.append(reinterpret_cast<const char*>(&p.first), sizeof(int16_t));
        out.append(reinterpret_cast<const char*>(&p.second), sizeof(int));
    }
    return out;
}

// Deserialize RLE pairs from string
std::vector<std::pair<int16_t, int>> SprintzCompressor::deserialize(const std::string& data) const {
    std::vector<std::pair<int16_t, int>> rle;
    size_t i = 0;
    while (i + sizeof(int16_t) + sizeof(int) <= data.size()) {
        int16_t val;
        int count;
        std::memcpy(&val, data.data() + i, sizeof(int16_t));
        std::memcpy(&count, data.data() + i + sizeof(int16_t), sizeof(int));
        rle.emplace_back(val, count);
        i += sizeof(int16_t) + sizeof(int);
    }
    return rle;
}

// Encode: double -> quantize -> delta -> RLE -> serialize
std::vector<std::string> SprintzCompressor::encode(const std::vector<double>& data) {
    if (data.empty()) return {};
    auto q = quantize(data, kScale);
    auto deltas = delta_encode(q);
    auto rle = rle_encode(deltas);
    std::string compressed = serialize(rle);
    return {compressed};
}

// Decode: deserialize -> RLE decode -> delta decode -> dequantize
std::vector<double> SprintzCompressor::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    auto rle = deserialize(encodedValues[0]);
    auto deltas = rle_decode(rle);
    auto q = delta_decode(deltas);
    return dequantize(q, kScale);
}
