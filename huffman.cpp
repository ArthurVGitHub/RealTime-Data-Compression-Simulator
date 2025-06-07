// huffman.cpp
// Huffman coding implementation adapted from https://github.com/steve-arnold/HuffmanCpp
// MIT License

#include "huffman.h"
#include <sstream>
#include <cstring>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <bitset>
#include <functional>

// --- Huffman Class Implementation ---

Huffman::Huffman()
        : alphabetcount(0), totalcharacters(0)
{
    leftpointer = nullptr;
    rightpointer = nullptr;
}

// --- In-Memory Encode/Decode for CompressorInterface ---
std::vector<std::string> Huffman::encode(const std::vector<double>& data) {
    if (data.empty()) {
        std::cerr << "Huffman: Input data vector is empty, skipping.\n";
        return {};
    }
    std::string input(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(double));
    std::set<unsigned char> unique_bytes(input.begin(), input.end());
    if (unique_bytes.size() < 2) {
        std::cerr << "Huffman: Input has fewer than 2 unique bytes, skipping.\n";
        return {};
    }
    std::string compressed = compress(input);
    return {compressed};
}



/*std::vector<std::string> Huffman::encode(const std::vector<double>& data) {
    // Convert double vector to byte string
    std::string input(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(double));
    std::string compressed = compress(input);
    return {compressed};
}*/

std::vector<double> Huffman::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    std::string decompressed = decompress(encodedValues[0]);
    size_t n = decompressed.size() / sizeof(double);
    std::vector<double> result(n);
    std::memcpy(result.data(), decompressed.data(), n * sizeof(double));
    return result;
}

// --- In-Memory Compress/Decompress API ---

std::string Huffman::compress(const std::string& input) {
    std::stringstream in(input);
    std::stringstream out;
    CompressFile(in, out);
    return out.str();
}

std::string Huffman::decompress(const std::string& input) {
    std::stringstream in(input);
    std::stringstream out;
    ExpandFile(in, out);
    return out.str();
}

// --- Core Huffman Implementation ---

bool Huffman::CompressFile(std::istream& fin, std::ostream& fout) {
    // Map symbols and build tree
    MapSymbols(fin);
    if (!GrowHuffmanTree()) return false;
    MakeCodesFromTree();
    WriteCompressedFileHeader(fout);
    WriteCompressedFile(fin, fout);
    return true;
}

bool Huffman::ExpandFile(std::istream& fin, std::ostream& fout) {
    if (!ReadCompressedFileHeader(fin)) return false;
    ReadCompressedFile(fin, fout);
    return true;
}

uintmax_t Huffman::MapSymbols(std::istream& fin) {
    symbolmap.clear();
    totalcharacters = 0;
    char c;
    while (fin.get(c)) {
        symbolmap[c]++;
        totalcharacters++;
    }
    fin.clear();
    fin.seekg(0, std::ios::beg);
    alphabetcount = static_cast<uint8_t>(symbolmap.size());
    return totalcharacters;
}

bool Huffman::GrowHuffmanTree() {
    while (!treeheap.empty()) treeheap.pop();
    for (auto& p : symbolmap) {
        treeheap.push(new TreeNode(p.first, p.second, true));
    }
    if (treeheap.empty()) return false;
    while (treeheap.size() > 1) {
        TreeNode* left = treeheap.top(); treeheap.pop();
        TreeNode* right = treeheap.top(); treeheap.pop();
        TreeNode* parent = new TreeNode('\0', left->weight + right->weight, false);
        parent->leftpointer = left;
        parent->rightpointer = right;
        treeheap.push(parent);
    }
    if (!treeheap.empty()) {
        leftpointer = treeheap.top();
        rightpointer = nullptr;
    }
    return true;
}

void Huffman::ClearSymbolMap() { symbolmap.clear(); }
void Huffman::ClearHuffmanTree() {
    // Recursively delete tree nodes
    std::function<void(TreeNode*)> deleteTree = [&](TreeNode* node) {
        if (!node) return;
        deleteTree(node->leftpointer);
        deleteTree(node->rightpointer);
        delete node;
    };
    deleteTree(leftpointer);
    leftpointer = nullptr;
    rightpointer = nullptr;
}
void Huffman::ClearCodeTable() { codetable.clear(); }

bool Huffman::GetSymbolMap(std::map<char, int>& out) {
    out = symbolmap;
    return !symbolmap.empty();
}

void Huffman::PrintCodeTable() {
    for (auto& tup : codetable) {
        unsigned char sym;
        int freq;
        std::string code;
        std::tie(sym, freq, code) = tup;
        std::cout << sym << ": " << code << " (" << freq << ")\n";
    }
}

uint16_t Huffman::GetAlphabetCount() { return alphabetcount; }
uintmax_t Huffman::GetTotalCharacters() { return totalcharacters; }
uintmax_t Huffman::GetTotalCodedBits() {
    uintmax_t total = 0;
    for (auto& tup : codetable) {
        int freq = std::get<1>(tup);
        total += freq * std::get<2>(tup).size();
    }
    return total;
}

// --- Huffman Coding Internals ---

void Huffman::MapSymbol(char c) { symbolmap[c]++; }

void Huffman::MakeCodesFromTree() {
    codetable.clear();
    if (leftpointer)
        MakePrefixCodes(leftpointer, "");
}

void Huffman::MakePrefixCodes(TreeNode* node, std::string prefix) {
    if (!node) return;
    if (node->isleaf) {
        codetable.emplace_back(static_cast<unsigned char>(node->symbol), node->weight, prefix);
    } else {
        MakePrefixCodes(node->leftpointer, prefix + "0");
        MakePrefixCodes(node->rightpointer, prefix + "1");
    }
}

void Huffman::SortCodeTable() {
    std::sort(codetable.begin(), codetable.end(),
              [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });
}

void Huffman::WriteCompressedFileHeader(std::ostream& fout) {
    // Write a simple header: alphabet size, symbol/freq pairs
    fout.write(filetag, 3);
    fout.put(alphabetcount);
    for (const auto& p : symbolmap) {
        fout.put(p.first);
        int freq = p.second;
        fout.write(reinterpret_cast<const char*>(&freq), sizeof(int));
    }
}

void Huffman::WriteCompressedFile(std::istream& fin, std::ostream& fout) {
    // Build symbol->code map
    std::map<unsigned char, std::string> codeMap;
    for (const auto& tup : codetable) {
        codeMap[std::get<0>(tup)] = std::get<2>(tup);
    }
    // Encode data
    std::string bitstring;
    char c;
    while (fin.get(c)) {
        bitstring += codeMap[static_cast<unsigned char>(c)];
    }
    // Pad to byte boundary
    int pad = (8 - (bitstring.size() % 8)) % 8;
    for (int i = 0; i < pad; ++i) bitstring += '0';
    fout.put(static_cast<char>(pad));
    // Write as bytes
    for (size_t i = 0; i < bitstring.size(); i += 8) {
        std::bitset<8> b(bitstring.substr(i, 8));
        fout.put(static_cast<char>(b.to_ulong()));
    }
}

bool Huffman::ReadCompressedFileHeader(std::istream& fin) {
    char tag[3];
    fin.read(tag, 3);
    if (std::memcmp(tag, filetag, 3) != 0) return false;
    alphabetcount = static_cast<uint8_t>(fin.get());
    symbolmap.clear();
    for (int i = 0; i < alphabetcount; ++i) {
        char sym = fin.get();
        int freq;
        fin.read(reinterpret_cast<char*>(&freq), sizeof(int));
        symbolmap[sym] = freq;
    }
    return GrowHuffmanTree() && (alphabetcount > 0);
}

void Huffman::ReadCompressedFile(std::istream& fin, std::ostream& fout) {
    // Build code->symbol map
    MakeCodesFromTree();
    std::map<std::string, unsigned char> decodeMap;
    for (const auto& tup : codetable) {
        decodeMap[std::get<2>(tup)] = std::get<0>(tup);
    }
    int pad = static_cast<unsigned char>(fin.get());
    std::string bitstring;
    char c;
    while (fin.get(c)) {
        std::bitset<8> b(static_cast<unsigned char>(c));
        bitstring += b.to_string();
    }
    if (pad > 0 && !bitstring.empty())
        bitstring.erase(bitstring.end() - pad, bitstring.end());
    // Decode
    std::string curr;
    for (char bit : bitstring) {
        curr += bit;
        auto it = decodeMap.find(curr);
        if (it != decodeMap.end()) {
            fout.put(static_cast<char>(it->second));
            curr.clear();
        }
    }
}
