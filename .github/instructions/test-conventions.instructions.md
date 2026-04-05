---
applyTo: "tests/**/*.cpp"
---

# PipeCAD 测试规范

## 测试框架选择

| 场景 | 框架 | CMake 链接目标 |
|------|------|----------------|
| 纯逻辑测试（无 Qt） | `GTest::Main` | `GTest::GTest GTest::Main` |
| 涉及 Qt 信号/QObject | `Qt6::Test + GTest` | `Qt6::Test GTest::GTest` |
| QML/渲染测试 | `Qt6::Test` | `Qt6::Quick Qt6::Test` |

## 文件命名

- 测试文件：`tests/test_<name>.cpp`
- 命名与 `tests/CMakeLists.txt` 中的 CMake 目标名保持一致

## 测试结构

```cpp
// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
// 按层引入被测头文件

TEST(FeatureGroup, SpecificBehavior) {
    // Arrange
    // Act
    // Assert
}
```

## 层对应链接规则

| 被测层 | 链接目标（CMakeLists.txt 中） |
|--------|-------------------------------|
| `lib/base/foundation` | `foundation` |
| `lib/platform/occt` | `geometry` |
| `apps/pipecad/model` | `model` |
| `apps/pipecad/engine` | `engine` |
| `lib/platform/vsg` | `visualization` |
| `lib/platform/vtk` | `vtk_visualization` |
| `lib/runtime/*` | `app` |
| `lib/framework/*` | `app` |

（旧别名仍有效，解析到新目标）

## 验收要求

- 每个任务（TX）需包含对应测试文件
- 新测试必须在 `tests/CMakeLists.txt` 注册
- `pixi run test` 必须全量通过（当前基线 46/46）

参见 [AGENTS.md](../../AGENTS.md) 测试章节
