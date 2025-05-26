#include "lzma_compressor.h"

#include <stdexcept>
#include <cstring>
#include <cmath>

extern "C" {
#include "lzma/C/LzmaEnc.h"
#include "lzma/C/LzmaDec.h"
#include "lzma/C/7zTypes.h"
}

void *SzAlloc(ISzAllocPtr, size_t size) { return malloc(size); }
void SzFree(ISzAllocPtr, void *address) { free(address); }
ISzAlloc g_Alloc = { SzAlloc, SzFree };

// Compress binaire data
std::vector<unsigned char> LzmaCompressor::compress(const std::vector<unsigned char>& data) {
    CLzmaEncProps props;
    LzmaEncProps_Init(&props);
    props.level = 5;
    props.dictSize = 1 << 20;

    unsigned char propsEncoded[LZMA_PROPS_SIZE];
    size_t propsSize = LZMA_PROPS_SIZE;

    size_t outLen = data.size() + data.size() / 3 + 128;
    std::vector<unsigned char> out(outLen + LZMA_PROPS_SIZE);

    size_t destLen = outLen;

    int res = LzmaEncode(
            out.data() + LZMA_PROPS_SIZE, &destLen,
            data.data(), data.size(),
            &props,
            propsEncoded, &propsSize,
            0, nullptr, &g_Alloc, &g_Alloc
    );
    if (res != SZ_OK)
        throw std::runtime_error("LZMA compressie mislukt");

    std::memcpy(out.data(), propsEncoded, LZMA_PROPS_SIZE);
    out.resize(LZMA_PROPS_SIZE + destLen);
    return out;
}

std::vector<unsigned char> LzmaCompressor::decompress(const std::vector<unsigned char>& compressed, size_t decompressedSize) {
    std::vector<unsigned char> out(decompressedSize);
    size_t outLen = decompressedSize;
    size_t srcLen = compressed.size() - LZMA_PROPS_SIZE;

    ELzmaStatus status;
    int res = LzmaDecode(
            out.data(), &outLen,
            compressed.data() + LZMA_PROPS_SIZE, &srcLen,
            compressed.data(), LZMA_PROPS_SIZE,
            LZMA_FINISH_ANY, &status, &g_Alloc
    );
    if (res != SZ_OK)
        throw std::runtime_error("LZMA decompressie mislukt");

    out.resize(outLen);
    return out;
}

// Voor jouw interface (encode/decode van doubles naar string)
std::vector<std::string> LzmaCompressor::encode(const std::vector<double>& data) {
    // Quantiseer double naar bytes
    std::vector<unsigned char> bytes(data.size() * sizeof(double));
    std::memcpy(bytes.data(), data.data(), bytes.size());
    auto compressed = compress(bytes);

    // Sla originele grootte op voor decompressie
    std::string result;
    uint32_t orig_size = static_cast<uint32_t>(bytes.size());
    result.append(reinterpret_cast<const char*>(&orig_size), sizeof(uint32_t));
    result.append(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    return {result};
}

std::vector<double> LzmaCompressor::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    const std::string& in = encodedValues[0];
    if (in.size() < sizeof(uint32_t)) return {};

    uint32_t orig_size = 0;
    std::memcpy(&orig_size, in.data(), sizeof(uint32_t));
    std::vector<unsigned char> compressed(in.begin() + sizeof(uint32_t), in.end());
    auto decompressed = decompress(compressed, orig_size);

    std::vector<double> out(decompressed.size() / sizeof(double));
    std::memcpy(out.data(), decompressed.data(), decompressed.size());
    return out;
}
