//
// Created by arthu on 22/05/2025.
//

#include "lz77.h"
#include <cmath>
#include <cstring>

// Quantize doubles to uint16_t
std::vector<uint16_t> Lz77::quantize(const std::vector<double>& data) const {
    std::vector<uint16_t> out(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        out[i] = static_cast<uint16_t>(std::round(data[i] * scale));
    return out;
}

std::vector<double> Lz77::dequantize(const std::vector<uint16_t>& data) const {
    std::vector<double> out(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        out[i] = static_cast<double>(data[i]) / scale;
    return out;
}

Lz77::Lz77(size_t window, size_t lookAhead)
        : windowSize(window), lookAheadBufferSize(lookAhead) {}

// LZ77 Encode
std::vector<std::string> Lz77::encode(const std::vector<double>& data) {
    if (data.empty()) return {};
    auto input = quantize(data);

    std::vector<Token> tokens;
    size_t pos = 0;
    while (pos < input.size()) {
        size_t bestOffset = 0, bestLength = 0;
        for (size_t w = (pos > windowSize ? pos - windowSize : 0); w < pos; ++w) {
            size_t length = 0;
            while (length < lookAheadBufferSize &&
                   pos + length < input.size() &&
                   input[w + length] == input[pos + length]) {
                ++length;
            }
            if (length > bestLength) {
                bestLength = length;
                bestOffset = pos - w;
            }
        }
        uint16_t next = (pos + bestLength < input.size()) ? input[pos + bestLength] : 0;
        tokens.push_back({bestOffset, bestLength, next});
        pos += bestLength + 1;
    }
    return {serialize(tokens)};
}

// LZ77 Decode
std::vector<double> Lz77::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    auto tokens = deserialize(encodedValues[0]);
    std::vector<uint16_t> output;
    for (const auto& token : tokens) {
        if (token.offset == 0 || token.length == 0) {
            output.push_back(token.next);
        } else {
            size_t start = output.size() - token.offset;
            for (size_t i = 0; i < token.length; ++i)
                output.push_back(output[start + i]);
            output.push_back(token.next);
        }
    }
    return dequantize(output);
}

// Serialization: Token -> string (binary)
std::string Lz77::serialize(const std::vector<Token>& tokens) const {
    std::string out;
    out.reserve(tokens.size() * (sizeof(size_t) * 2 + sizeof(uint16_t)));
    for (const auto& t : tokens) {
        out.append(reinterpret_cast<const char*>(&t.offset), sizeof(size_t));
        out.append(reinterpret_cast<const char*>(&t.length), sizeof(size_t));
        out.append(reinterpret_cast<const char*>(&t.next), sizeof(uint16_t));
    }
    return out;
}

// Deserialization: string -> Token
std::vector<Lz77::Token> Lz77::deserialize(const std::string& data) const {
    std::vector<Token> tokens;
    size_t i = 0;
    while (i + 2 * sizeof(size_t) + sizeof(uint16_t) <= data.size()) {
        Token t;
        std::memcpy(&t.offset, data.data() + i, sizeof(size_t));
        std::memcpy(&t.length, data.data() + i + sizeof(size_t), sizeof(size_t));
        std::memcpy(&t.next, data.data() + i + 2 * sizeof(size_t), sizeof(uint16_t));
        tokens.push_back(t);
        i += 2 * sizeof(size_t) + sizeof(uint16_t);
    }
    return tokens;
}
