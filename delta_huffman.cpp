//
// Created by arthu on 19/05/2025.
//

#include "delta_huffman.h"
#include <cstring>
#include <iostream>

std::vector<std::string> DeltaHuffman::encode(const std::vector<double>& data) {
    // 1. Delta encode
    std::vector<int64_t> deltas = delta.encode(data);
    if (deltas.empty()) {
        std::cerr << "DeltaHuffman: Delta vector is empty, skipping.\n";
        return {};
    }
    // 2. Serialize deltas to bytes
    std::string delta_bytes(reinterpret_cast<const char*>(deltas.data()), deltas.size() * sizeof(int64_t));
    // 3. Huffman compress
    std::set<unsigned char> unique_bytes(delta_bytes.begin(), delta_bytes.end());
    if (unique_bytes.size() < 2) {
        std::cerr << "DeltaHuffman: Delta bytes have fewer than 2 unique values, skipping.\n";
        return {};
    }
    std::string compressed = huffman.compress(delta_bytes);
    if (compressed.empty()) {
        std::cerr << "DeltaHuffman: Huffman compression returned empty, skipping.\n";
        return {};
    }
    return {compressed};
}

/*std::vector<std::string> DeltaHuffman::encode(const std::vector<double>& data) {
    // 1. Delta encode
    std::vector<int64_t> deltas = delta.encode(data);
    // 2. Serialize deltas to bytes
    std::string delta_bytes(reinterpret_cast<const char*>(deltas.data()), deltas.size() * sizeof(int64_t));
    // 3. Huffman compress
    std::string compressed = huffman.compress(delta_bytes);
    return {compressed};
}*/

std::vector<double> DeltaHuffman::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    // 1. Huffman decompress
    std::string decompressed = huffman.decompress(encodedValues[0]);
    // 2. Deserialize bytes to deltas
    size_t n = decompressed.size() / sizeof(int64_t);
    std::vector<int64_t> deltas(n);
    std::memcpy(deltas.data(), decompressed.data(), n * sizeof(int64_t));
    // 3. Delta decode
    return delta.decode(deltas);
}
