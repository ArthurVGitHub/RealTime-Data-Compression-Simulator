//
// Created by arthu on 19/05/2025.
//

#ifndef THESIS_DELTA_RLE_H
#define THESIS_DELTA_RLE_H
#pragma once
#include "compressor_interface.h"
#include "delta.h"
#include "rle.h"
#include <vector>
#include <string>

class DeltaRle : public CompressorInterface {
public:
    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;
private:
    Delta delta;
    Rle rle;
};

#endif //THESIS_DELTA_RLE_H
