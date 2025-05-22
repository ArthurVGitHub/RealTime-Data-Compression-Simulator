//
// Created by arthu on 22/05/2025.
//

#ifndef THESIS_SPRINTZ_H
#define THESIS_SPRINTZ_H
#pragma once
#include "compressor_interface.h"
#include <vector>
#include <string>
#include <cstdint>

class SprintzCompressor : public CompressorInterface {
public:
    SprintzCompressor() = default;

    // Encode: input doubles, output vector with one string (serialized compressed data)
    std::vector<std::string> encode(const std::vector<double>& data) override;

    // Decode: input vector with one string, output decoded doubles
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;

private:
    // Helper: quantize doubles to int16_t for Sprintz (lossy, but typical for Sprintz)
    std::vector<int16_t> quantize(const std::vector<double>& data, double scale = 1000.0) const;
    std::vector<double> dequantize(const std::vector<int16_t>& data, double scale = 1000.0) const;

    // Delta encode/decode
    std::vector<int16_t> delta_encode(const std::vector<int16_t>& data) const;
    std::vector<int16_t> delta_decode(const std::vector<int16_t>& deltas) const;

    // RLE encode/decode (for zeros)
    std::vector<std::pair<int16_t, int>> rle_encode(const std::vector<int16_t>& deltas) const;
    std::vector<int16_t> rle_decode(const std::vector<std::pair<int16_t, int>>& rle) const;

    // Serialize/deserialize RLE pairs to/from a string
    std::string serialize(const std::vector<std::pair<int16_t, int>>& rle) const;
    std::vector<std::pair<int16_t, int>> deserialize(const std::string& data) const;

    static constexpr double kScale = 1000.0; // quantization scale
};

#endif //THESIS_SPRINTZ_H
