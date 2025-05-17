#ifndef THESIS_LZ4_H
#define THESIS_LZ4_H
#pragma once
#include <vector>
#include <string>

class LZ4 {
public:
    // Compresses a vector of doubles to a vector of char (bytes)
    static std::vector<char> compress(const std::vector<double>& input);

    // Decompresses a vector of char (bytes) to a vector of doubles
    static std::vector<double> decompress(const std::vector<char>& compressed, size_t originalSize);
};

#endif //THESIS_LZ4_H
