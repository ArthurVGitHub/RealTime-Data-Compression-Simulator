#include "lz4_compresor.h"
#include "lz4/lz4.h"
#include <cstring>
#include <stdexcept>
#include <iostream>

std::vector<std::string> LZ4Compressor::encode(const std::vector<double>& data) {
    if (data.empty()) return {""};
    const char* src = reinterpret_cast<const char*>(data.data());
    int srcSize = static_cast<int>(data.size() * sizeof(double));
    int maxDstSize = LZ4_compressBound(srcSize);

    std::string out;
    out.resize(sizeof(uint64_t) + maxDstSize);

    uint64_t origNumDoubles = data.size();
    std::memcpy(&out[0], &origNumDoubles, sizeof(uint64_t));

    int compressedSize = LZ4_compress_default(
            src, &out[sizeof(uint64_t)], srcSize, maxDstSize
    );
    if (compressedSize <= 0) throw std::runtime_error("LZ4 compression failed");

    out.resize(sizeof(uint64_t) + compressedSize);
    return {out};
}

std::vector<double> LZ4Compressor::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty() || encodedValues[0].size() < sizeof(uint64_t)) return {};
    const std::string& in = encodedValues[0];

    uint64_t origNumDoubles = 0;
    std::memcpy(&origNumDoubles, in.data(), sizeof(uint64_t));

    std::vector<double> output(origNumDoubles);
    int expectedSize = static_cast<int>(origNumDoubles * sizeof(double));
    int compressedSize = static_cast<int>(in.size() - sizeof(uint64_t));

    int decompressedSize = LZ4_decompress_safe(
            in.data() + sizeof(uint64_t),
            reinterpret_cast<char*>(output.data()),
            compressedSize,
            expectedSize
    );
    if (decompressedSize != expectedSize) {
        std::cerr << "LZ4 decompressed size mismatch: got " << decompressedSize << ", expected " << expectedSize << std::endl;
        throw std::runtime_error("LZ4 decompression failed");
    }
    return output;
}
