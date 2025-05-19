//
// Created by arthu on 19/05/2025.
//

#ifndef THESIS_RLE_H
#define THESIS_RLE_H
#pragma once
#include <vector>
#include <cstdint>
#include <string>

class Rle {
public:
    // Encodes a vector of int64_t into (value, count) pairs and serializes to a string
    std::string encode(const std::vector<int64_t>& data) const;

    // Decodes a string of (value, count) pairs back to a vector of int64_t
    std::vector<int64_t> decode(const std::string& encoded) const;
};

#endif //THESIS_RLE_H
