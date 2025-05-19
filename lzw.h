#ifndef THESIS_LZW_H
#define THESIS_LZW_H
#pragma once
#include <vector>
#include <string>
#include "compressor_interface.h"

class Lzw : public CompressorInterface {
public:
    Lzw() = default;
    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;

    std::string compress(const std::string& input) const { return lzw_compress(input); }
    std::string decompress(const std::string& input) const { return lzw_decompress(input); }

private:
    std::string lzw_compress(const std::string& input) const;
    std::string lzw_decompress(const std::string& input) const;
};

#endif //THESIS_LZW_H
