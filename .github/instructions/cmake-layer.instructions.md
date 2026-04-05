---
applyTo: "**/CMakeLists.txt"
---

# PipeCAD CMake 层规范

## 构建入口（唯一真相）

只有以下目录是实际 CMake 构建入口，**旧目录为迁移镜像，不可新增源文件**：

| 新构建入口 | 对应旧镜像（只读） |
|------------|-------------------|
| `src/lib/base/foundation/` | `src/foundation/` |
| `src/lib/platform/occt/geometry/` | `src/geometry/` |
| `src/lib/platform/vsg/visualization/` | `src/visualization/` |
| `src/lib/platform/vtk/vtk-visualization/` | `src/vtk-visualization/` |
| `src/lib/runtime/{app,command,task}/` | `src/app/` `src/command/` |
| `src/lib/framework/app/` | — |
| `src/apps/pipecad/{model,engine,workbench,ui}/` | `src/model/` `src/engine/` `src/ui/` |

## 目标别名（保持兼容）

新目标名 → 旧别名（tests/ 和旧代码中可继续使用别名）：

```
lib_base        → foundation
lib_platform_occt → geometry
pipecad_app_model → model
pipecad_app_engine → engine
lib_platform_vsg  → visualization
lib_platform_vtk  → vtk_visualization
lib_runtime / lib_framework → app
```

## 依赖链（禁止逆向依赖）

```
lib_base
  └─► lib_platform_occt
        └─► pipecad_app_model
              └─► pipecad_app_engine
lib_platform_vsg (depends on lib_platform_occt)
lib_platform_vtk (depends on lib_platform_occt)
lib_runtime ─► lib_framework ─► pipecad_lib (interface)
  └─► pipecad_app_{model,engine,workbench,ui,resources}
        └─► pipecad_app ─► pipecad (executable)
```

## 新目标模板

```cmake
add_library(my_new_target STATIC
    src/MyClass.cpp
)
target_include_directories(my_new_target PUBLIC .)
target_link_libraries(my_new_target PUBLIC lib_base)  # 按层填写
set_target_properties(my_new_target PROPERTIES CXX_STANDARD 17)
```

## 可执行路径

```
build/debug/src/apps/pipecad/pipecad     # Debug
build/release/src/apps/pipecad/pipecad   # Release
```

参见 [架构文档](../../docs/architecture.md)
