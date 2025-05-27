#include "lz77_compressor.h"
#include "lz77/yalz77-master/lz77.h"
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <iostream>

std::vector<std::string> Lz77Compressor::encode(const std::vector<double>& data) {
    if (data.empty()) return {""};
    const char* src = reinterpret_cast<const char*>(data.data());
    size_t srcSize = data.size() * sizeof(double);
    std::string input(src, src + srcSize);

    lz77::compress_t compress;
    std::string compressed = compress.feed(input);

    //std::cout << "[LZ77] Encoding: input bytes = " << srcSize
    //          << ", compressed bytes = " << compressed.size() << std::endl;

    std::string out(sizeof(uint64_t), '\0');
    uint64_t numDoubles = data.size();
    std::memcpy(&out[0], &numDoubles, sizeof(uint64_t));
    out += compressed;
    return {out};
}

std::vector<double> Lz77Compressor::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty() || encodedValues[0].size() < sizeof(uint64_t)) return {};
    const std::string& in = encodedValues[0];

    uint64_t numDoubles = 0;
    std::memcpy(&numDoubles, in.data(), sizeof(uint64_t));
    size_t expectedBytes = numDoubles * sizeof(double);

    // Decompress using yalz77
    lz77::decompress_t decompress;
    std::string temp;
    decompress.feed(in.substr(sizeof(uint64_t)), temp);
    const std::string& decompressed = decompress.result();

    //std::cout << "[LZ77] Decompressed bytes: " << decompressed.size()
    //          << ", expected: " << expectedBytes << std::endl;

/*    if (decompressed.size() != expectedBytes) {
        std::cerr << "Decompression failed: expected " << expectedBytes
                  << " bytes, got " << decompressed.size() << std::endl;
        return {};
    }*/

    std::vector<double> out(numDoubles);
    if (expectedBytes > 0)
        std::memcpy(out.data(), decompressed.data(), expectedBytes);
    return out;
}
