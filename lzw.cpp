#include "lzw.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>

std::string Lzw::lzw_compress(const std::string& input) const {
    std::unordered_map<std::string, int> dict;
    for (int i = 0; i < 256; ++i)
        dict[std::string(1, i)] = i;

    std::string w;
    std::vector<int> output;
    int dictSize = 256;

    for (char c : input) {
        std::string wc = w + c;
        if (dict.count(wc)) {
            w = wc;
        } else {
            output.push_back(dict[w]);
            dict[wc] = dictSize++;
            w = std::string(1, c);
        }
    }
    if (!w.empty())
        output.push_back(dict[w]);

    // Write codes as binary (2 bytes per code)
    std::string compressed;
    for (int code : output) {
        compressed.push_back(static_cast<char>((code >> 8) & 0xFF));
        compressed.push_back(static_cast<char>(code & 0xFF));
    }
    return compressed;
}

std::string Lzw::lzw_decompress(const std::string& input) const {
    std::vector<int> codes;
    for (size_t i = 0; i + 1 < input.size(); i += 2) {
        int code = (static_cast<unsigned char>(input[i]) << 8) |
                   static_cast<unsigned char>(input[i + 1]);
        codes.push_back(code);
    }

    if (codes.empty()) return ""; // <-- Add this line!

    std::unordered_map<int, std::string> dict;
    for (int i = 0; i < 256; ++i)
        dict[i] = std::string(1, static_cast<unsigned char>(i));

    std::string w(1, static_cast<unsigned char>(codes[0]));
    std::string result = w;
    int dictSize = 256;
    for (size_t i = 1; i < codes.size(); ++i) {
        int k = codes[i];
        std::string entry;
        if (dict.count(k)) {
            entry = dict[k];
        } else if (k == dictSize) {
            entry = w + w[0];
        } else {
            throw std::runtime_error("Invalid LZW code.");
        }
        result += entry;
        dict[dictSize++] = w + entry[0];
        w = entry;
    }
    return result;
}

std::vector<std::string> Lzw::encode(const std::vector<double>& data) {
    std::string input(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(double));
    std::string compressed = lzw_compress(input);
    return {compressed};
}

std::vector<double> Lzw::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    std::string decompressed = lzw_decompress(encodedValues[0]);
    size_t count = decompressed.size() / sizeof(double);
    std::vector<double> result(count);
    std::memcpy(result.data(), decompressed.data(), count * sizeof(double));
    return result;
}
