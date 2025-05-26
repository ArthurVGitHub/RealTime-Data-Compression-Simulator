//
// Created by arthu on 26/05/2025.
//

#include "zstd_compressor.h"
#include <zstd.h>
#include <cstring>
#include <cmath>

ZstdCompressor::ZstdCompressor(int level) : compressionLevel(level) {}

std::vector<int16_t> ZstdCompressor::quantize(const std::vector<double>& data) const {
    std::vector<int16_t> out(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        out[i] = static_cast<int16_t>(std::round(data[i] * kScale));
    return out;
}

std::vector<double> ZstdCompressor::dequantize(const std::vector<int16_t>& data) const {
    std::vector<double> out(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        out[i] = static_cast<double>(data[i]) / kScale;
    return out;
}

// ENCODE
std::vector<std::string> ZstdCompressor::encode(const std::vector<double>& data) {
    if (data.empty()) return {};
    size_t nBytes = data.size() * sizeof(double);
    const char* rawData = reinterpret_cast<const char*>(data.data());

    size_t maxDstSize = ZSTD_compressBound(nBytes);
    std::string compressed(maxDstSize, '\0');

    size_t cSize = ZSTD_compress(
            &compressed[0], maxDstSize,
            rawData, nBytes,
            compressionLevel
    );
    if (ZSTD_isError(cSize)) return {};

    std::string result;
    uint32_t nSamples = static_cast<uint32_t>(data.size());
    result.append(reinterpret_cast<const char*>(&nSamples), sizeof(uint32_t));
    result.append(compressed.data(), cSize);
    return {result};
}

// DECODE
std::vector<double> ZstdCompressor::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    const std::string& in = encodedValues[0];
    if (in.size() < sizeof(uint32_t)) return {};

    uint32_t nSamples = 0;
    std::memcpy(&nSamples, in.data(), sizeof(uint32_t));
    size_t cSize = in.size() - sizeof(uint32_t);

    std::vector<double> output(nSamples);
    size_t dSize = ZSTD_decompress(
            output.data(), nSamples * sizeof(double),
            in.data() + sizeof(uint32_t), cSize
    );
    if (ZSTD_isError(dSize) || dSize != nSamples * sizeof(double)) return {};

    return output;
}
