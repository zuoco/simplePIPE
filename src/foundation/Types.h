// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>
#include <variant>
#include <cstdint>
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace foundation {

// ============================================================
// UUID — 随机 v4
// ============================================================

struct UUID {
    uint8_t data[16]{};

    bool operator==(const UUID& o) const {
        for (int i = 0; i < 16; ++i) if (data[i] != o.data[i]) return false;
        return true;
    }
    bool operator!=(const UUID& o) const { return !(*this == o); }
    bool operator<(const UUID& o) const {
        for (int i = 0; i < 16; ++i) {
            if (data[i] < o.data[i]) return true;
            if (data[i] > o.data[i]) return false;
        }
        return false;
    }

    std::string toString() const {
        std::ostringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < 16; ++i) {
            ss << std::setw(2) << static_cast<int>(data[i]);
            if (i == 3 || i == 5 || i == 7 || i == 9) ss << '-';
        }
        return ss.str();
    }

    static UUID generate() {
        static thread_local std::mt19937 rng{std::random_device{}()};
        static thread_local std::uniform_int_distribution<uint8_t> dist;
        UUID uuid;
        for (auto& b : uuid.data) b = dist(rng);
        // RFC 4122 v4: version bits
        uuid.data[6] = (uuid.data[6] & 0x0F) | 0x40;
        // variant bits
        uuid.data[8] = (uuid.data[8] & 0x3F) | 0x80;
        return uuid;
    }

    bool isNull() const {
        for (auto b : data) if (b != 0) return false;
        return true;
    }
};

// ============================================================
// Variant — double / int / string / enum (int)
// ============================================================

using Variant = std::variant<double, int, std::string>;

inline double variantToDouble(const Variant& v) {
    if (auto* d = std::get_if<double>(&v)) return *d;
    if (auto* i = std::get_if<int>(&v)) return static_cast<double>(*i);
    throw std::bad_variant_access{};
}

inline int variantToInt(const Variant& v) {
    if (auto* i = std::get_if<int>(&v)) return *i;
    if (auto* d = std::get_if<double>(&v)) return static_cast<int>(*d);
    throw std::bad_variant_access{};
}

inline const std::string& variantToString(const Variant& v) {
    if (auto* s = std::get_if<std::string>(&v)) return *s;
    throw std::bad_variant_access{};
}

// ============================================================
// 单位枚举
// ============================================================

enum class UnitSystem {
    SI,       // 公制 (mm / MPa / ℃)
    Imperial  // 英制 (inch / psi / ℉)
};

enum class LengthUnit {
    Millimeter,
    Meter,
    Inch,
    Foot
};

enum class PressureUnit {
    MPa,
    PSI,
    Bar
};

} // namespace foundation
