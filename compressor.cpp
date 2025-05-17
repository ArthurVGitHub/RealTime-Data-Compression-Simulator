//
// Created by arthu on 12/05/2025.
//

#include "compressor.h"
#include <cmath>

std::vector<double> compressor::deltaEncodeNext(const std::vector<double>& input) {
    auto start_time = std::chrono::system_clock::now();

    if (nextIndex >= static_cast<int>(input.size())) {
        deltaEncoded.push_back(std::numeric_limits<double>::quiet_NaN());
        auto end_time = std::chrono::system_clock::now();
        durationCompression += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
        return deltaEncoded;
    }

    double encodedValue;
    if (nextIndex == 0) {
        encodedValue = input[0];
    } else {
        encodedValue = input[nextIndex] - input[nextIndex - 1];
    }

    nextIndex++;
    auto end_time = std::chrono::system_clock::now();

    deltaEncoded.push_back(encodedValue);
    durationCompression += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

    return deltaEncoded;
}

std::vector<double> compressor::deltaDecode(const std::vector<double>& encodedValues) {
    std::vector<double> decodedValues;
    double prevValue = 0.0;

    for (double delta : encodedValues) {
        if (std::isnan(delta)) break;
        double originalValue = prevValue + delta;
        decodedValues.push_back(originalValue);
        prevValue = originalValue;
    }

    return decodedValues;
}

double compressor::getCompressionDuration() const {
    return durationCompression.count() / 1e6; // milliseconds
}

double compressor::getCompressionRate(const std::vector<double>& original, const std::vector<double>& encoded) const {
    return static_cast<double>(encoded.size()) / original.size();
}
