//
// Created by arthu on 18/05/2025.
//

#ifndef THESIS_COMPRESSOR_FACTORY_H
#define THESIS_COMPRESSOR_FACTORY_H
#pragma once
#include <memory>
#include <string>
#include "drh.h"
#include "delta.h"
#include "huffman.h"
#include "lzw.h"
// Add new algorithms

inline std::unique_ptr<CompressorInterface> createCompressor(const std::string& name) {
    if (name == "DRH") return std::make_unique<DRH>();
    if (name == "Delta") return std::make_unique<Delta>();
    if(name == "Huffman") return std::make_unique<Huffman>();
    if (name == "LZW") return std::make_unique<Lzw>();
    // Add more algorithms here
    throw std::runtime_error("Unknown algorithm: " + name);
}

#endif //THESIS_COMPRESSOR_FACTORY_H
