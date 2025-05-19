//
// Created by arthu on 19/05/2025.
//

#include "rle.h"
#include <cstring>

std::string Rle::encode(const std::vector<int64_t>& data) const {
    std::string out;
    if (data.empty()) return out;

    int64_t current = data[0];
    int count = 1;
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] == current && count < INT32_MAX) {
            ++count;
        } else {
            out.append(reinterpret_cast<const char*>(&current), sizeof(int64_t));
            out.append(reinterpret_cast<const char*>(&count), sizeof(int));
            current = data[i];
            count = 1;
        }
    }
    // Write last run
    out.append(reinterpret_cast<const char*>(&current), sizeof(int64_t));
    out.append(reinterpret_cast<const char*>(&count), sizeof(int));
    return out;
}

std::vector<int64_t> Rle::decode(const std::string& encoded) const {
    std::vector<int64_t> out;
    size_t pos = 0;
    while (pos + sizeof(int64_t) + sizeof(int) <= encoded.size()) {
        int64_t value;
        int count;
        std::memcpy(&value, encoded.data() + pos, sizeof(int64_t));
        pos += sizeof(int64_t);
        std::memcpy(&count, encoded.data() + pos, sizeof(int));
        pos += sizeof(int);
        out.insert(out.end(), count, value);
    }
    return out;
}
