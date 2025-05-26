#ifndef THESIS_SPRINTZ_H
#define THESIS_SPRINTZ_H

#include <vector>
#include <string>
#include <cstdint>
#include "compressor_interface.h"

class SprintzCompressor : public CompressorInterface {
public:
    SprintzCompressor();

    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;

    static constexpr size_t kBlockSize = 8;

private:
    std::vector<int16_t> quantize(const std::vector<double>& data);
    std::vector<double> dequantize(const std::vector<int16_t>& data) const;

    double dynamic_scale_;
    int16_t last_value_;
    bool first_block_;
};

#endif // THESIS_SPRINTZ_H