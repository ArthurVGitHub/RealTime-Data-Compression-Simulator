//
// Created by arthu on 18/05/2025.
//

#ifndef THESIS_COMPRESSOR_FACTORY_H
#define THESIS_COMPRESSOR_FACTORY_H
#pragma once
#include <memory>
#include <string>
#include "drh.h"
#include "huffman.h"
#include "lzw.h"
#include "delta_rle.h"
#include "delta_huffman.h"
#include "delta_lzw.h"
#include "sprintz.h"
#include "lz77_compressor.h"
#include "lz4_compresor.h"
#include "zstd_compressor.h"
#include "lzma_compressor.h"
#include "lzo_compressor.h"
#include "snappy_compressor.h"
#include "rle.h"
// Add new algorithms

inline std::unique_ptr<CompressorInterface> createCompressor(const std::string& name) {
    if (name == "DRH") return std::make_unique<DRH>();
    if (name == "Delta(+RLE)") return std::make_unique<DeltaRle>();
    if(name == "Huffman") return std::make_unique<Huffman>();
    if (name == "LZW") return std::make_unique<Lzw>();
    if (name == "Delta+Huffman") return std::make_unique<DeltaHuffman>();
    if (name == "Delta+LZW") return std::make_unique<DeltaLzw>();
    if (name == "Sprintz") return std::make_unique<SprintzCompressor>();
    if (name == "LZ77") return std::make_unique<Lz77Compressor>();
    if (name == "LZ4") return std::make_unique<LZ4Compressor>();
    if (name == "Zstd") return std::make_unique<ZstdCompressor>();
    if (name == "LZMA") return std::make_unique<LzmaCompressor>();
    if (name == "LZO") return std::make_unique<LzoCompressor>();
    if (name == "Snappy") return std::make_unique<SnappyCompressor>();
    if (name == "(SW-)RLE") return std::make_unique<Rle>();

    // Add more algorithms here
    throw std::runtime_error("Unknown algorithm: " + name);
}

#endif //THESIS_COMPRESSOR_FACTORY_H
