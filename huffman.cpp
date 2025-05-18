//
// Created by arthu on 18/05/2025.
//

#include "huffman.h"
#include <bitset>
#include <sstream>
#include <cmath>
#include <cstdint>

Huffman::Huffman(int digitsAfterComma)
        : digitsAfterComma(digitsAfterComma) {}

std::vector<std::string> Huffman::encode(const std::vector<double>& data) {
    std::vector<std::string> encoded;
    if (data.empty()) return encoded;

    for (size_t i = 0; i < data.size(); ++i) {
        int deltaValue = static_cast<int>(std::round(data[i] * std::pow(10, digitsAfterComma)));
        std::string encodedValue;

        if (i == 0) {
            // First value: store as 16-bit binary
            encodedValue = std::bitset<16>(static_cast<uint16_t>(deltaValue)).to_string();
        } else if (deltaValue == 0) {
            encodedValue = "1";
        } else {
            int absDelta = std::abs(deltaValue);
            std::string sign = (deltaValue < 0) ? "00" : "01";
            int codeLength = absDelta - 1;
            encodedValue = "0";
            for (int j = 0; j < codeLength; ++j)
                encodedValue += "1";
            encodedValue += sign;
        }
        encoded.push_back(encodedValue);
    }
    return encoded;
}

std::vector<double> Huffman::decode(const std::vector<std::string>& encodedValues) {
    std::vector<double> decoded;
    if (encodedValues.empty()) return decoded;

    for (size_t i = 0; i < encodedValues.size(); ++i) {
        const std::string& code = encodedValues[i];

        if (i == 0) {
            // First value: 16-bit binary to int
            int x = std::stoi(code, nullptr, 2);
            decoded.push_back(x / std::pow(10, digitsAfterComma));
        } else if (code[0] == '1') {
            decoded.push_back(0.0);
        } else {
            // Count number of '1's after the first '0'
            int codeLength = 1;
            while (codeLength < static_cast<int>(code.size()) && code[codeLength] == '1')
                ++codeLength;
            int absDelta = codeLength;
            std::string sign = code.substr(code.size() - 2);
            int deltaValue = (sign == "01") ? absDelta : -absDelta;
            decoded.push_back(deltaValue / std::pow(10, digitsAfterComma));
        }
    }
    return decoded;
}
