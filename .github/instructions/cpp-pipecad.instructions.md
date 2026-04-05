---
applyTo: "src/**/*.{cpp,h,hpp,cppm}"
---

# PipeCAD C++ 编码规范

## 内存管理

- **OCCT 对象**：使用 `Handle<T>`，禁止裸指针指向 `Standard_Transient` 子类
- **VSG 对象**：使用 `vsg::ref_ptr<T>`
- **VTK 对象**：使用 `vtkSmartPointer<T>`
- **信号/槽**：使用 `foundation::Signal<T>`，不依赖 Qt 的 Q_OBJECT

## 命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | PascalCase | `PipePoint`, `ComponentCatalog` |
| 函数/方法 | camelCase | `setType()`, `deriveParams()` |
| 成员变量 | `m_` 前缀 | `m_type`, `m_pipeSpec` |
| 私有成员 | 可加 `_` 后缀 | `typeParams_` |
| 常量 | `kPascalCase` 或 `UPPER_SNAKE` | `kDefaultRadius` |
| 枚举成员 | PascalCase | `PipePointType::Bend` |

## 几何精度

- 几何比较阈值必须使用 `Precision::Confusion()`，禁止硬编码 `1e-7`
- 角度精度使用 `Precision::Angular()`

## 异常处理

- OCCT 异常捕获用 `Standard_Failure`，不用 `std::exception`：
  ```cpp
  try { ... }
  catch (const Standard_Failure& e) { /* e.GetMessageString() */ }
  ```

## 线程安全

- OCCT **非线程安全**，多线程访问 OCCT 对象必须加锁
- 后台任务使用 `WorkerGroup` 调度，通过 `DocumentSnapshot` 只读访问文档

## 文件头

```cpp
// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
```

## 架构边界（禁止跨层引用）

- `src/lib/base/` — 不得依赖 OCCT/VSG/Qt
- `src/lib/platform/occt/` — 不得依赖 VSG/VTK/Qt
- `src/apps/pipecad/model/` — 不得依赖 engine/ui 层

参见 [架构文档](../../docs/architecture.md)
