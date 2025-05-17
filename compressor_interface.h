#ifndef COMPRESSOR_INTERFACE_H
#define COMPRESSOR_INTERFACE_H

#include <vector>
#include <string>

class CompressorInterface {
public:
    virtual ~CompressorInterface() = default;
    virtual std::vector<std::string> encode(const std::vector<double>& data) = 0;
    virtual std::vector<double> decode(const std::vector<std::string>& encodedValues) = 0;
};

#endif // COMPRESSOR_INTERFACE_H
