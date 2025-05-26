#include "snappy_compressor.h"
#include <snappy.h>
#include "snappy/snappy.h"
#include <cstring>
#include <cstdint>
#include <stdexcept>

std::string SnappyCompressor::compress(const std::string& input) {
    std::string output;
    snappy::Compress(input.data(), input.size(), &output);
    return output;
}

std::string SnappyCompressor::decompress(const std::string& input) {
    std::string output;
    if (!snappy::Uncompress(input.data(), input.size(), &output))
        throw std::runtime_error("Snappy decompression failed");
    return output;
}

std::vector<std::string> SnappyCompressor::encode(const std::vector<double>& data) {
    std::string input(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(double));
    std::string compressed = compress(input);

    // Store original size for correct decompression
    std::string result;
    uint32_t orig_size = static_cast<uint32_t>(input.size());
    result.append(reinterpret_cast<const char*>(&orig_size), sizeof(uint32_t));
    result.append(compressed);
    return {result};
}

std::vector<double> SnappyCompressor::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    const std::string& in = encodedValues[0];
    if (in.size() < sizeof(uint32_t)) return {};

    uint32_t orig_size = 0;
    std::memcpy(&orig_size, in.data(), sizeof(uint32_t));
    std::string compressed = in.substr(sizeof(uint32_t));
    std::string decompressed = decompress(compressed);

    if (decompressed.size() != orig_size) throw std::runtime_error("Snappy decompressed size mismatch");
    std::vector<double> out(decompressed.size() / sizeof(double));
    std::memcpy(out.data(), decompressed.data(), decompressed.size());
    return out;
}
