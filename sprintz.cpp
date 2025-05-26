#include "sprintz.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <cstring>

SprintzCompressor::SprintzCompressor()
        : dynamic_scale_(1000.0),
          last_value_(0),
          first_block_(true) {}

std::vector<int16_t> SprintzCompressor::quantize(const std::vector<double>& data) {
    // Auto-detect optimal scale factor
    double max_val = *std::max_element(data.begin(), data.end(),
                                       [](double a, double b) { return std::abs(a) < std::abs(b); });

    dynamic_scale_ = std::max(1000.0, 32767.0 / (std::abs(max_val) + 1e-9));

    std::vector<int16_t> out(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        double scaled = data[i] * dynamic_scale_;
        scaled = std::max<double>(scaled, std::numeric_limits<int16_t>::min());
        scaled = std::min<double>(scaled, std::numeric_limits<int16_t>::max());
        out[i] = static_cast<int16_t>(std::round(scaled));
    }
    return out;
}

std::vector<double> SprintzCompressor::dequantize(const std::vector<int16_t>& data) const {
    std::vector<double> out(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        out[i] = static_cast<double>(data[i]) / dynamic_scale_;
    }
    return out;
}

std::vector<std::string> SprintzCompressor::encode(const std::vector<double>& data) {
    auto quantized = quantize(data);
    std::vector<uint8_t> compressed;
    std::vector<uint8_t> bitwidths;
    std::vector<uint32_t> block_sizes;

    // Pad data to complete blocks
    size_t padded_size = ((data.size() + kBlockSize - 1) / kBlockSize) * kBlockSize;
    quantized.resize(padded_size, last_value_);

    for (size_t i = 0; i < padded_size; i += kBlockSize) {
        std::vector<int16_t> block(quantized.begin() + i,
                                   quantized.begin() + i + kBlockSize);
        std::vector<int16_t> residuals(kBlockSize);

        // Delta encoding with overflow protection
        residuals[0] = block[0] - last_value_;
        if (residuals[0] > 32767 || residuals[0] < -32768) {
            residuals[0] = 0;
            last_value_ = block[0];
        }

        for (size_t j = 1; j < kBlockSize; j++) {
            residuals[j] = block[j] - block[j-1];
        }
        last_value_ = block.back();

        // Offset to unsigned range
        int16_t min_residual = *std::min_element(residuals.begin(), residuals.end());
        for (auto& r : residuals) r -= min_residual;

        // Calculate bitwidth with safety margin
        uint16_t max_residual = *std::max_element(residuals.begin(), residuals.end());
        uint8_t bw = std::min(16,
                              static_cast<int>(std::ceil(std::log2(max_residual + 1))) + 1);

        // Store metadata
        bitwidths.push_back(bw);
        block_sizes.push_back(kBlockSize);

        // Store min residual (2 bytes)
        compressed.push_back(static_cast<uint8_t>(min_residual & 0xFF));
        compressed.push_back(static_cast<uint8_t>((min_residual >> 8) & 0xFF));

        // Bitpack residuals
        uint32_t buffer = 0;
        int bits = 0;
        for (int16_t val : residuals) {
            buffer |= (static_cast<uint32_t>(val) << bits);
            bits += bw;
            while (bits >= 8) {
                compressed.push_back(buffer & 0xFF);
                buffer >>= 8;
                bits -= 8;
            }
        }
        if (bits > 0) {
            compressed.push_back(buffer & 0xFF);
        }
    }

    // Prepare output
    std::string result;
    uint32_t n_samples = static_cast<uint32_t>(data.size());
    uint32_t n_blocks = static_cast<uint32_t>(bitwidths.size());

    // Metadata header
    result.append(reinterpret_cast<const char*>(&n_samples), sizeof(uint32_t));
    result.append(reinterpret_cast<const char*>(&n_blocks), sizeof(uint32_t));
    result.append(reinterpret_cast<const char*>(&dynamic_scale_), sizeof(double));

    // Bitwidths and block sizes
    result.append(reinterpret_cast<const char*>(bitwidths.data()), bitwidths.size());
    result.append(reinterpret_cast<const char*>(block_sizes.data()),
                  block_sizes.size() * sizeof(uint32_t));

    // Compressed data
    result.append(reinterpret_cast<const char*>(compressed.data()), compressed.size());

    first_block_ = false;
    return {result};
}

std::vector<double> SprintzCompressor::decode(const std::vector<std::string>& encodedValues) {
    if (encodedValues.empty()) return {};
    const std::string& data = encodedValues[0];
    if (data.size() < 2 * sizeof(uint32_t) + sizeof(double)) return {};

    size_t pos = 0;
    uint32_t n_samples, n_blocks;
    double scale_factor;

    std::memcpy(&n_samples, data.data() + pos, sizeof(uint32_t)); pos += sizeof(uint32_t);
    std::memcpy(&n_blocks, data.data() + pos, sizeof(uint32_t)); pos += sizeof(uint32_t);
    std::memcpy(&scale_factor, data.data() + pos, sizeof(double)); pos += sizeof(double);
    dynamic_scale_ = scale_factor;

    if (n_blocks == 0) return std::vector<double>(n_samples, 0.0);

    // Read bitwidths and block sizes
    std::vector<uint8_t> bitwidths(n_blocks);
    std::vector<uint32_t> block_sizes(n_blocks);

    std::memcpy(bitwidths.data(), data.data() + pos, n_blocks); pos += n_blocks;
    std::memcpy(block_sizes.data(), data.data() + pos,
                n_blocks * sizeof(uint32_t)); pos += n_blocks * sizeof(uint32_t);

    // Process blocks
    std::vector<int16_t> quantized;
    const uint8_t* comp_data = reinterpret_cast<const uint8_t*>(data.data() + pos);
    size_t comp_size = data.size() - pos;
    size_t comp_pos = 0;

    for (size_t b = 0; b < n_blocks; b++) {
        if (comp_pos + 2 > comp_size) break;

        // Read min residual
        int16_t min_residual = static_cast<int16_t>(comp_data[comp_pos]) |
                               (static_cast<int16_t>(comp_data[comp_pos + 1]) << 8);
        comp_pos += 2;

        // Bit unpack
        std::vector<int16_t> residuals(block_sizes[b]);
        uint32_t buffer = 0;
        int bits = 0;
        size_t data_pos = 0;
        size_t bytes_needed = (bitwidths[b] * block_sizes[b] + 7) / 8;

        for (size_t i = 0; i < block_sizes[b]; i++) {
            while (bits < bitwidths[b] && data_pos < bytes_needed) {
                buffer |= static_cast<uint32_t>(comp_data[comp_pos + data_pos]) << bits;
                bits += 8;
                data_pos++;
            }
            residuals[i] = static_cast<int16_t>(buffer & ((1u << bitwidths[b]) - 1));
            buffer >>= bitwidths[b];
            bits -= bitwidths[b];
            residuals[i] += min_residual; // Apply offset
        }
        comp_pos += data_pos;

        // Reconstruct block
        if (first_block_ && b == 0) {
            quantized.push_back(residuals[0] + last_value_);
            for (size_t i = 1; i < residuals.size(); i++) {
                quantized.push_back(quantized.back() + residuals[i]);
            }
            first_block_ = false;
        } else {
            for (size_t i = 0; i < residuals.size(); i++) {
                quantized.push_back(quantized.back() + residuals[i]);
            }
        }
        last_value_ = quantized.back();
    }

    // Trim to original size and convert
    quantized.resize(n_samples);
    return dequantize(quantized);
}