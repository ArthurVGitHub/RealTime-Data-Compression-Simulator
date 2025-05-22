//
// Created by arthu on 22/05/2025.
//

#ifndef THESIS_LZ77_H
#define THESIS_LZ77_H
#pragma once
#include "compressor_interface.h"
#include <vector>
#include <string>
#include <cstdint>

class Lz77 : public CompressorInterface {
public:
    Lz77(size_t windowSize = 32, size_t lookAheadBufferSize = 16);

    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;

private:
    size_t windowSize;
    size_t lookAheadBufferSize;
    double scale = 1000.0; // quantization scale for doubles

    // Helper: quantize/dequantize
    std::vector<uint16_t> quantize(const std::vector<double>& data) const;
    std::vector<double> dequantize(const std::vector<uint16_t>& data) const;

    // Internal representation for LZ77 tokens
    struct Token {
        size_t offset;
        size_t length;
        uint16_t next;
    };

    // Serialization helpers
    std::string serialize(const std::vector<Token>& tokens) const;
    std::vector<Token> deserialize(const std::string& data) const;
};

#endif //THESIS_LZ77_H
