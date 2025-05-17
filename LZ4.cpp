#include "LZ4.h"
#include <cstring>

std::vector<char> LZ4::compress(const std::vector<double>& input) {
    const char* src = reinterpret_cast<const char*>(input.data());
    int srcSize = static_cast<int>(input.size() * sizeof(double));
    int maxDstSize = LZ4_compressBound(srcSize);

    std::vector<char> compressed(maxDstSize);
    int compressedSize = LZ4_compress_default(src, compressed.data(), srcSize, maxDstSize);
    if (compressedSize <= 0) return {};

    compressed.resize(compressedSize);
    return compressed;
}

std::vector<double> LZ4::decompress(const std::vector<char>& compressed, size_t originalSize) {
    std::vector<double> output(originalSize);
    int decompressedSize = LZ4_decompress_safe(
            compressed.data(),
            reinterpret_cast<char*>(output.data()),
            static_cast<int>(compressed.size()),
            static_cast<int>(originalSize * sizeof(double))
    );
    if (decompressedSize < 0) return {};
    return output;
}
