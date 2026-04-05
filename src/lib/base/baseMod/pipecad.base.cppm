// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
//
// T63: lib/base 总模块接口 — 聚合所有 lib/base 子模块
// 使用者可以 `import pipecad.base;` 一次性获得全部基础类型

export module pipecad.base;

// 导出子模块（re-export）
export import pipecad.base.math;
export import pipecad.base.types;
