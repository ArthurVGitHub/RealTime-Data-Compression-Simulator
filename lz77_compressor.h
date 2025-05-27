#ifndef THESIS_LZ77_COMPRESSOR_H
#define THESIS_LZ77_COMPRESSOR_H

#include "compressor_interface.h"
#include <vector>
#include <string>
#include <cstdint>

class Lz77Compressor : public CompressorInterface {
public:
    Lz77Compressor() = default;

    // Encodes a vector of doubles to a compressed string (using yalz77)
    std::vector<std::string> encode(const std::vector<double>& data) override;

    // Decodes a compressed string back to a vector of doubles
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;
};

#endif // THESIS_LZ77_COMPRESSOR_H
