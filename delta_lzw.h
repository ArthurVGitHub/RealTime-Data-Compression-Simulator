//
// Created by arthu on 19/05/2025.
//

#ifndef THESIS_DELTA_LZW_H
#define THESIS_DELTA_LZW_H
#pragma once
#include "compressor_interface.h"
#include "delta.h"
#include "lzw.h"   // Your LZW compressor class
#include <vector>
#include <string>

class DeltaLzw : public CompressorInterface {
public:
    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;
private:
    Delta delta;
    Lzw lzw;
};


#endif //THESIS_DELTA_LZW_H
