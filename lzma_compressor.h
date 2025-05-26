#ifndef LZMA_COMPRESSOR_H
#define LZMA_COMPRESSOR_H

#include "compressor_interface.h"
#include <vector>
#include <cstddef>

class LzmaCompressor : public CompressorInterface {
public:
    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;

    // Extra: voor binaire data
    std::vector<unsigned char> compress(const std::vector<unsigned char>& data);
    std::vector<unsigned char> decompress(const std::vector<unsigned char>& compressed, size_t decompressedSize);
};

#endif
