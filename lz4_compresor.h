#pragma once
#include "compressor_interface.h"
#include <vector>
#include <string>
#include <cstdint>

class LZ4Compressor : public CompressorInterface {
public:
    LZ4Compressor() = default;
    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;
private:
    double scale = 1000.0; // quantization scale for doubles

    std::vector<int16_t> quantize(const std::vector<double>& data) const;
    std::vector<double> dequantize(const std::vector<int16_t>& data) const;
};
