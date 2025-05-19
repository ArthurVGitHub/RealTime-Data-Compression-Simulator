//
// Created by arthu on 19/05/2025.
//

#ifndef THESIS_DELTA_HUFFMAN_H
#define THESIS_DELTA_HUFFMAN_H
#pragma once
#include "compressor_interface.h"
#include "delta.h"
#include "huffman.h"
#include <vector>
#include <string>

class DeltaHuffman : public CompressorInterface {
public:
    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;
private:
    Delta delta;
    Huffman huffman;
};

#endif //THESIS_DELTA_HUFFMAN_H
