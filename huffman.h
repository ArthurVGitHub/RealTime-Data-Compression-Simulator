#ifndef THESIS_HUFFMAN_H
#define THESIS_HUFFMAN_H
#pragma once

#include <vector>
#include <string>
#include <map>
#include <queue>
#include <cstdint>
#include "compressor_interface.h"

// Huffman coding implementation adapted from https://github.com/steve-arnold/HuffmanCpp
// MIT License

const char filetag[] = "SAC";

struct TreeNode
{
    bool isleaf;
    char symbol;
    int weight;
    TreeNode* leftpointer;
    TreeNode* rightpointer;
    TreeNode(char symbol, int weight, bool isleaf = false)
            : isleaf(isleaf), symbol(symbol), weight(weight), leftpointer(nullptr), rightpointer(nullptr) {}
};

class Huffman : public CompressorInterface
{
public:
    Huffman();

    std::vector<std::string> encode(const std::vector<double>& data) override;
    std::vector<double> decode(const std::vector<std::string>& encodedValues) override;

    std::string compress(const std::string& input);
    std::string decompress(const std::string& input);

    // File-based API (optional, can be private)
    bool CompressFile(std::istream& fin, std::ostream& fout);
    bool ExpandFile(std::istream& fin, std::ostream& fout);
    uintmax_t MapSymbols(std::istream&);
    bool GrowHuffmanTree();
    void ClearSymbolMap();
    void ClearHuffmanTree();
    void ClearCodeTable();
    bool GetSymbolMap(std::map<char, int>&);
    void PrintCodeTable();
    uint16_t GetAlphabetCount();
    uintmax_t GetTotalCharacters();
    uintmax_t GetTotalCodedBits();

private:
    void MapSymbol(char);
    void MakeCodesFromTree();
    void MakePrefixCodes(TreeNode*, std::string);
    void SortCodeTable();
    void WriteCompressedFileHeader(std::ostream&);
    void WriteCompressedFile(std::istream&, std::ostream&);
    bool ReadCompressedFileHeader(std::istream&);
    void ReadCompressedFile(std::istream&, std::ostream&);

    struct compare
    {
        bool operator()(TreeNode* leftnode, TreeNode* rightnode)
        {
            return (leftnode->weight > rightnode->weight);
        }
    };

    std::map<char, int> symbolmap;
    std::priority_queue<TreeNode*, std::vector<TreeNode*>, compare> treeheap;
    std::vector<std::tuple<unsigned char, int, std::string>> codetable;
    TreeNode* leftpointer = nullptr;
    TreeNode* rightpointer = nullptr;
    uint8_t alphabetcount = 0;
    uintmax_t totalcharacters = 0;
};

#endif //THESIS_HUFFMAN_H
