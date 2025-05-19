//
// Created by arthu on 19/05/2025.
//

#include "delta_lzw.h"
#include <cstring>
std::vector<std::string> DeltaLzw::encode(const std::vector<double>& data) {
    std::vector<int64_t> deltas = delta.encode(data);
    std::string delta_bytes(reinterpret_cast<const char*>(deltas.data()), deltas.size() * sizeof(int64_t));
    std::string compressed = lzw.compress(delta_bytes);
    return {compressed};
}

std::vector<double> DeltaLzw::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    std::string decompressed = lzw.decompress(encodedValues[0]);
    size_t n = decompressed.size() / sizeof(int64_t);
    std::vector<int64_t> deltas(n);
    std::memcpy(deltas.data(), decompressed.data(), n * sizeof(int64_t));
    return delta.decode(deltas);
}
