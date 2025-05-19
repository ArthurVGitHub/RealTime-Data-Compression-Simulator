//
// Created by arthu on 17/05/2025.
//

#ifndef THESIS_DELTA_H
#define THESIS_DELTA_H
#include <vector>
#include <cstdint>
#include <string>
#include "compressor_interface.h"
//combined with RLE actually
class Delta{
public:
    Delta() = default;

    std::vector<int64_t> encode(const std::vector<double>& data) const;
    std::vector<double> decode(const std::vector<int64_t>& deltas) const;
};


#endif //THESIS_DELTA_H
