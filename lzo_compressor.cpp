#include "lzo_compressor.h"
#include <cstdint>

extern "C" {
#include "lzo/lzo-2.10/include/lzo/lzo1x.h"
#include "lzo/lzo-2.10/include/lzo/lzoutil.h"
#include "lzo/lzo-2.10/include/lzo/lzoconf.h"

}


#include <stdexcept>
#include <cstring>
#include <iostream>

std::vector<unsigned char> LzoCompressor::compress(const std::vector<unsigned char>& data) {
    if (lzo_init() != LZO_E_OK)
        throw std::runtime_error("LZO init failed");

    size_t out_len = data.size() + data.size() / 16 + 64 + 3;
    std::vector<unsigned char> out(out_len);

    // Werkt alleen als data.size() < 2GB
    std::vector<unsigned char> wrkmem(LZO1X_1_MEM_COMPRESS);

    lzo_uint dst_len = out_len;
    int r = lzo1x_1_compress(
            data.data(), static_cast<lzo_uint>(data.size()),
            out.data(), &dst_len,
            wrkmem.data()
    );
    if (r != LZO_E_OK)
        throw std::runtime_error("LZO compressie mislukt");

    out.resize(dst_len);
    return out;
}

std::vector<unsigned char> LzoCompressor::decompress(const std::vector<unsigned char>& compressed, size_t orig_size) {
    std::vector<unsigned char> out(orig_size);
    lzo_uint dst_len = orig_size;

    int r = lzo1x_decompress(
            compressed.data(), static_cast<lzo_uint>(compressed.size()),
            out.data(), &dst_len,
            nullptr
    );
    if (r != LZO_E_OK)
        throw std::runtime_error("LZO decompressie mislukt");

    out.resize(dst_len);
    return out;
}

// Voor je interface:
std::vector<std::string> LzoCompressor::encode(const std::vector<double>& data) {
    std::vector<unsigned char> bytes(data.size() * sizeof(double));
    std::memcpy(bytes.data(), data.data(), bytes.size());
    auto compressed = compress(bytes);

    std::string result;
    uint32_t orig_size = static_cast<uint32_t>(bytes.size());
    result.append(reinterpret_cast<const char*>(&orig_size), sizeof(uint32_t));
    result.append(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    return {result};
}

std::vector<double> LzoCompressor::decode(const std::vector<std::string>& encodedValues) {
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
