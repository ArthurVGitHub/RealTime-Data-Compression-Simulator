#ifndef SNAPPY_COMPRESSOR_H
#define SNAPPY_COMPRESSOR_H

#include "compressor_interface.h"
#include <vector>
#include <string>

class SnappyCompressor : public CompressorInterface {
public:
    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;

    std::string compress(const std::string& input);
    std::string decompress(const std::string& input);
};

#endif
