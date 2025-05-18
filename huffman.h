//
// Created by arthu on 18/05/2025.
//

#ifndef THESIS_HUFFMAN_H
#define THESIS_HUFFMAN_H
#pragma once
#include <vector>
#include <string>
#include <cmath>
#include "compressor_interface.h"

class Huffman : public CompressorInterface {
public:
    explicit Huffman(int digitsAfterComma = 3); // Default precision: 3
    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;

private:
    int digitsAfterComma;
};

#endif //THESIS_HUFFMAN_H
