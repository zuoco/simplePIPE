// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
//
// T63: lib/base 第一批模块接口单元 — 基础类型模块
// 包装 foundation/Types.h 中的 UUID 与 Variant
// 注意：使用 pipecad:: 包装命名空间导出以避免与全局模块片段冲突

module;
// 全局模块片段：引入遗留头文件
#include "foundation/Types.h"

export module pipecad.base.types;

// 通过 pipecad:: 命名空间导出，避免与全局模块片段 foundation:: 发生重声明冲突
export namespace pipecad {
    using UUID    = ::foundation::UUID;
    using Variant = ::foundation::Variant;
    inline double variantToDouble(const ::foundation::Variant& v) {
        return ::foundation::variantToDouble(v);
    }
    inline int variantToInt(const ::foundation::Variant& v) {
        return ::foundation::variantToInt(v);
    }
}
