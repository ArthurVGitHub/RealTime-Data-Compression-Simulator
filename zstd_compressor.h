#ifndef ZSTD_COMPRESSOR_H
#define ZSTD_COMPRESSOR_H

#include "compressor_interface.h"
#include <vector>
#include <string>
#include <cstdint>

class ZstdCompressor : public CompressorInterface {
public:
    ZstdCompressor(int compressionLevel = 3);

    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;

private:
    int compressionLevel;
    static constexpr double kScale = 1000.0; // For quantization (optional)
    std::vector<int16_t> quantize(const std::vector<double>& data) const;
    std::vector<double> dequantize(const std::vector<int16_t>& data) const;
};

#endif // ZSTD_COMPRESSOR_H
