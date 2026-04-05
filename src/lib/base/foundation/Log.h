// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdio>
#include <ctime>

// ============================================================
// 日志宏 — 输出到 stdout，级别: DEBUG / INFO / WARN / ERROR
// 后续可替换为真正的日志库（spdlog 等）
// ============================================================

namespace foundation {
namespace log {

enum class Level { Debug = 0, Info, Warn, Error };

// 全局最低日志级别（可在应用初始化时修改）
inline Level& minLevel() {
    static Level lvl = Level::Debug;
    return lvl;
}

inline const char* levelStr(Level l) {
    switch (l) {
        case Level::Debug: return "DEBUG";
        case Level::Info:  return "INFO ";
        case Level::Warn:  return "WARN ";
        case Level::Error: return "ERROR";
    }
    return "?????";
}

inline void log(Level level, const char* file, int line, const char* msg) {
    if (level < minLevel()) return;
    std::fprintf(stdout, "[%s] %s:%d — %s\n", levelStr(level), file, line, msg);
    std::fflush(stdout);
}

} // namespace log
} // namespace foundation

// 宏接口（file/line 自动注入）
#define LOG_DEBUG(msg) ::foundation::log::log(::foundation::log::Level::Debug, __FILE__, __LINE__, (msg))
#define LOG_INFO(msg)  ::foundation::log::log(::foundation::log::Level::Info,  __FILE__, __LINE__, (msg))
#define LOG_WARN(msg)  ::foundation::log::log(::foundation::log::Level::Warn,  __FILE__, __LINE__, (msg))
#define LOG_ERROR(msg) ::foundation::log::log(::foundation::log::Level::Error, __FILE__, __LINE__, (msg))
