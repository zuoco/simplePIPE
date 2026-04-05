// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
//
// T63: lib/base 第一批模块接口单元 — 数学工具模块
// 包装 foundation/Math.h 中的 Vec3 与角度转换工具
// 注意：使用 pipecad:: 包装命名空间导出以保持一致

module;
// 全局模块片段：引入遗留头文件
#include "foundation/Math.h"

export module pipecad.base.math;

// 通过 pipecad:: 命名空间导出
export namespace pipecad {
    using Vec3 = ::foundation::math::Vec3;
    constexpr double PI = ::foundation::math::PI;
    inline double degToRad(double deg) { return ::foundation::math::degToRad(deg); }
    inline double radToDeg(double rad) { return ::foundation::math::radToDeg(rad); }
}
