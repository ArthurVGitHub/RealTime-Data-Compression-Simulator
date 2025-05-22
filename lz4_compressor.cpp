#include "lz4_compresor.h"
#include "lz4/lz4.h"
#include <cstring>
#include <cmath>
#include <stdexcept>

// Quantize doubles to int16_t
std::vector<int16_t> LZ4Compressor::quantize(const std::vector<double>& data) const {
    std::vector<int16_t> out(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        out[i] = static_cast<int16_t>(std::round(data[i] * scale));
    return out;
}

std::vector<double> LZ4Compressor::dequantize(const std::vector<int16_t>& data) const {
    std::vector<double> out(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        out[i] = static_cast<double>(data[i]) / scale;
    return out;
}

std::vector<std::string> LZ4Compressor::encode(const std::vector<double>& data) {
    if (data.empty()) return {};
    auto q = quantize(data);

    // Compress as raw bytes
    const char* src = reinterpret_cast<const char*>(q.data());
    int srcSize = static_cast<int>(q.size() * sizeof(int16_t));
    int maxDstSize = LZ4_compressBound(srcSize);
    std::string compressed(maxDstSize, '\0');

    int compressedSize = LZ4_compress_default(src, &compressed[0], srcSize, maxDstSize);
    if (compressedSize <= 0) throw std::runtime_error("LZ4 compression failed");

    // Store original size for decompression (first 4 bytes)
    std::string out(sizeof(int), '\0');
    std::memcpy(&out[0], &srcSize, sizeof(int));
    out.append(compressed.data(), compressedSize);

    return {out};
}

std::vector<double> LZ4Compressor::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    const std::string& in = encodedValues[0];

    if (in.size() < sizeof(int)) throw std::runtime_error("LZ4: input too small");

    int srcSize = 0;
    std::memcpy(&srcSize, in.data(), sizeof(int));
    if (srcSize <= 0) throw std::runtime_error("LZ4: bad stored size");

    std::vector<int16_t> q(srcSize / sizeof(int16_t));
    int decompressedSize = LZ4_decompress_safe(
            in.data() + sizeof(int),
            reinterpret_cast<char*>(q.data()),
            static_cast<int>(in.size() - sizeof(int)),
            srcSize
    );
    if (decompressedSize != srcSize) throw std::runtime_error("LZ4 decompression failed");

    return dequantize(q);
}
