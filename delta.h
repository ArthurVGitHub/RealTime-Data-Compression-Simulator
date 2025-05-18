//
// Created by arthu on 17/05/2025.
//

#ifndef THESIS_DELTA_H
#define THESIS_DELTA_H
#include <vector>
#include <cstdint>
#include <string>
#include "compressor_interface.h"
class Delta : public CompressorInterface{
public:
    Delta();

    // Encode and decode interface
    std::vector<std::string> encode(const std::vector<double>& data);
    std::vector<double> decode(const std::vector<std::string>& encodedValues);

private:
    std::vector<int64_t> deltaEncodeLossless(const std::vector<double>& data) const;
    std::vector<double> deltaDecodeLossless(const std::vector<int64_t>& deltas) const;

    // Conversion helpers
    uint64_t doubleToUint64(double d) const;
    double uint64ToDouble(uint64_t u) const;
};


#endif //THESIS_DELTA_H
