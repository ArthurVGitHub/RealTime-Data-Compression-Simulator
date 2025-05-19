//
// Created by arthu on 19/05/2025.
//

#include "delta_rle.h"
#include <cstring>

std::vector<std::string> DeltaRle::encode(const std::vector<double>& data) {
    // 1. Delta encode
    std::vector<int64_t> deltas = delta.encode(data);
    // 2. RLE encode and serialize
    std::string serialized = rle.encode(deltas);
    return {serialized};
}

std::vector<double> DeltaRle::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    // 1. RLE decode
    std::vector<int64_t> deltas = rle.decode(encodedValues[0]);
    // 2. Delta decode
    return delta.decode(deltas);
}
