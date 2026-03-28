#pragma once

#include "foundation/Types.h"

#include <cctype>
#include <string>

namespace ui {

inline int hexValue(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }
    return -1;
}

inline bool uuidFromString(const std::string& text, foundation::UUID& out)
{
    std::string hex;
    hex.reserve(32);

    for (char c : text) {
        if (c == '-') {
            continue;
        }
        if (hexValue(c) < 0) {
            return false;
        }
        hex.push_back(c);
    }

    if (hex.size() != 32) {
        return false;
    }

    for (int i = 0; i < 16; ++i) {
        const int hi = hexValue(hex[static_cast<std::size_t>(i) * 2]);
        const int lo = hexValue(hex[static_cast<std::size_t>(i) * 2 + 1]);
        if (hi < 0 || lo < 0) {
            return false;
        }
        out.data[i] = static_cast<uint8_t>((hi << 4) | lo);
    }
    return true;
}

} // namespace ui
