#include "drh.h"
#include <cstring>
#include <queue>
#include <map>
#include <set>
#include <iostream>
#include <functional>
#include <cmath>

// ======= Bitwise double <-> uint64_t helpers =======
uint64_t doubleToUint64(double d) {
    uint64_t u;
    std::memcpy(&u, &d, sizeof(double));
    return u;
}

double uint64ToDouble(uint64_t u) {
    double d;
    std::memcpy(&d, &u, sizeof(double));
    return d;
}

// ======= DELTA ENCODING (lossless, bitwise) =======
std::vector<int64_t> DRH::deltaEncodeLossless(const std::vector<double>& data) const {
    std::vector<int64_t> deltas;
    uint64_t prev = doubleToUint64(data[0]);
    deltas.push_back(static_cast<int64_t>(prev));
    for (size_t i = 1; i < data.size(); ++i) {
        uint64_t curr = doubleToUint64(data[i]);
        deltas.push_back(static_cast<int64_t>(curr) - static_cast<int64_t>(prev));
        prev = curr;
    }
    return deltas;
}

std::vector<double> DRH::deltaDecodeLossless(const std::vector<int64_t>& deltas) const {
    std::vector<double> data;
    int64_t curr = deltas[0];
    data.push_back(uint64ToDouble(static_cast<uint64_t>(curr)));
    for (size_t i = 1; i < deltas.size(); ++i) {
        curr += deltas[i];
        data.push_back(uint64ToDouble(static_cast<uint64_t>(curr)));
    }
    return data;
}

// ======= RLE ENCODING =======
std::vector<std::pair<int64_t, int>> DRH::rleEncode(const std::vector<int64_t>& deltas) const {
    std::vector<std::pair<int64_t, int>> rle;
    int64_t prev = deltas[0];
    int count = 1;
    for (size_t i = 1; i < deltas.size(); ++i) {
        if (deltas[i] == prev) {
            ++count;
        } else {
            rle.emplace_back(prev, count);
            prev = deltas[i];
            count = 1;
        }
    }
    rle.emplace_back(prev, count);
    return rle;
}

std::vector<int64_t> DRH::rleDecode(const std::vector<std::pair<int64_t, int>>& rleData) const {
    std::vector<int64_t> deltas;
    for (const auto& p : rleData) {
        for (int i = 0; i < p.second; ++i) {
            deltas.push_back(p.first);
        }
    }
    return deltas;
}

// ======= HUFFMAN ENCODING =======
struct HuffmanNode {
    int64_t value;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode(int64_t v, int f) : value(v), freq(f), left(nullptr), right(nullptr) {}
    HuffmanNode(HuffmanNode* l, HuffmanNode* r) : value(0), freq(l->freq + r->freq), left(l), right(r) {}
    ~HuffmanNode() { delete left; delete right; }
};

struct CompareNode {
    bool operator()(HuffmanNode* a, HuffmanNode* b) { return a->freq > b->freq; }
};

void DRH::buildHuffmanTree(const std::vector<std::pair<int64_t, int>>& rleData) {
    std::map<int64_t, int> freq;
    for (const auto& p : rleData) {
        freq[p.first] += p.second;
    }
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, CompareNode> pq;
    for (const auto& f : freq) {
        pq.push(new HuffmanNode(f.first, f.second));
    }
    if (pq.empty()) return;
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        pq.push(new HuffmanNode(left, right));
    }
    HuffmanNode* root = pq.top();

    valueToHuffmanCode.clear();
    huffmanCodeToValue.clear();
    std::function<void(HuffmanNode*, std::string)> build = [&](HuffmanNode* node, std::string code) {
        if (!node->left && !node->right) {
            valueToHuffmanCode[node->value] = code.empty() ? "0" : code;
            huffmanCodeToValue[code.empty() ? "0" : code] = node->value;
            return;
        }
        if (node->left) build(node->left, code + "0");
        if (node->right) build(node->right, code + "1");
    };
    build(root, "");
    delete root;
}

std::string DRH::huffmanEncode(const std::vector<std::pair<int64_t, int>>& rleData) const {
    std::string bitstring;
    for (const auto& p : rleData) {
        auto it = valueToHuffmanCode.find(p.first);
        if (it == valueToHuffmanCode.end()) {
            std::cerr << "Warning: value " << p.first << " not found in Huffman code map.\n";
            continue; // or throw, depending on your needs
        }
        std::string code = it->second;
        for (int i = 0; i < p.second; ++i) {
            bitstring += code;
        }
    }
    return bitstring;
}

std::vector<std::pair<int64_t, int>> DRH::huffmanDecode(const std::string& bitstring) const {
    std::vector<std::pair<int64_t, int>> rleData;
    size_t i = 0;
    while (i < bitstring.size()) {
        std::string code;
        size_t j = i;
        while (j < bitstring.size()) {
            code += bitstring[j];
            auto it = huffmanCodeToValue.find(code);
            if (it != huffmanCodeToValue.end()) {
                rleData.emplace_back(it->second, 1);
                i = j + 1;
                break;
            }
            ++j;
        }
        if (j == bitstring.size()) break;
    }
    // Combine consecutive same values into RLE
    std::vector<std::pair<int64_t, int>> rleCombined;
    if (!rleData.empty()) {
        int64_t prev = rleData[0].first;
        int count = 1;
        for (size_t k = 1; k < rleData.size(); ++k) {
            if (rleData[k].first == prev) {
                ++count;
            } else {
                rleCombined.emplace_back(prev, count);
                prev = rleData[k].first;
                count = 1;
            }
        }
        rleCombined.emplace_back(prev, count);
    }
    return rleCombined;
}

// ======= ENCODE/DECODE =======
DRH::DRH() {}

std::vector<std::string> DRH::encode(const std::vector<double>& data) {
    if (data.empty()) return {};
    auto deltas = deltaEncodeLossless(data);
    auto rle = rleEncode(deltas);
    buildHuffmanTree(rle);
    std::string bitstring = huffmanEncode(rle);
    return {bitstring};
}

std::vector<double> DRH::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    auto rle = huffmanDecode(encodedValues[0]);
    auto deltas = rleDecode(rle);
    return deltaDecodeLossless(deltas);
}
