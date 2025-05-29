#ifndef THESIS_RLE_H
#define THESIS_RLE_H

#include <vector>
#include <string>
#include <cstdint>
#include "Compressor_Interface.h"
//SW-RLE with the combination of the moving window in CompressorRunner
class Rle : public CompressorInterface {
public:
    // For general use (double, lossless), otherwise decoding less precise and you don't get true
    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encoded) override;

    // For DeltaRle (int64_t, efficient)
    std::string encode(const std::vector<int64_t>& data) const;
    std::vector<int64_t> decode(const std::string& encoded) const;
};

#endif // THESIS_RLE_H
