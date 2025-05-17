#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <vector>
#include <chrono>
#include <limits>

class compressor {
public:
    compressor() : nextIndex(0), durationCompression(0) {}

    std::vector<double> deltaEncodeNext(const std::vector<double>& input);
    std::vector<double> deltaDecode(const std::vector<double>& encodedValues);
    double getCompressionDuration() const;
    double getCompressionRate(const std::vector<double>& original, const std::vector<double>& encoded) const;

private:
    int nextIndex;
    std::vector<double> deltaEncoded;
    std::chrono::nanoseconds durationCompression;
};

#endif