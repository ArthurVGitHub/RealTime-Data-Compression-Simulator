#pragma once
#include <vector>
#include <string>
#include <map>
#include <cstdint>

class DRH {
public:
    DRH(); // No need for digitsAfterComma anymore

    std::vector<std::string> encode(const std::vector<double>& data);
    std::vector<double> decode(const std::vector<std::string>& encodedValues);

private:
    // Lossless delta encode/decode on bit patterns
    std::vector<int64_t> deltaEncodeLossless(const std::vector<double>& data) const;
    std::vector<double> deltaDecodeLossless(const std::vector<int64_t>& deltas) const;

    // RLE
    std::vector<std::pair<int64_t, int>> rleEncode(const std::vector<int64_t>& deltas) const;
    std::vector<int64_t> rleDecode(const std::vector<std::pair<int64_t, int>>& rleData) const;

    // Huffman
    void buildHuffmanTree(const std::vector<std::pair<int64_t, int>>& rleData);
    std::string huffmanEncode(const std::vector<std::pair<int64_t, int>>& rleData) const;
    std::vector<std::pair<int64_t, int>> huffmanDecode(const std::string& bitstring) const;

    // Huffman maps
    std::map<int64_t, std::string> valueToHuffmanCode;
    std::map<std::string, int64_t> huffmanCodeToValue;
};
