# M1 构建重编组

## T53 设计 lib/apps 顶层 CMake 拓扑

**目标**
将当前 `src` 目录下的目标组织方式重设计为 `lib/apps` 双根结构。

**输入**
- `src/CMakeLists.txt`
- `src/*/CMakeLists.txt`
- T50-T52 产出

**改动范围**
- `src/CMakeLists.txt`
- 顶层构建拓扑设计

**交付物**
- 新的目标依赖图
- 过渡阶段的 `add_subdirectory` 方案
- 架构静态库与 app 目标装配方案

**依赖**
- T50
- T51
- T52

**验收标准**
- 存在统一架构库目标
- 存在单个 app 业务目标与可执行目标

## T54 解除 RecomputeEngine 源文件级拼接

**目标**
消除当前 app 静态库直接吞并 `RecomputeEngine.cpp` 的结构性阻塞。

**输入**
- `src/app/CMakeLists.txt`
- `src/engine/RecomputeEngine.h`
- `src/engine/RecomputeEngine.cpp`
- T53 产出

**改动范围**
- 构建系统
- 重算链路的链接设计

**交付物**
- 新的重算链接方案
- 替代性的目标依赖或接口依赖设计

**依赖**
- T53

**验收标准**
- 不再存在源文件级跨目录拼接
- 重算链路仍可正常构建

## T55 设计过渡兼容层

**目标**
在不一次性改完所有 include 的前提下，提供平滑迁移路径。

**输入**
- T53 产出
- 现有 include 结构

**改动范围**
- 兼容头规划
- include 路径迁移策略

**交付物**
- 兼容 include 规则
- 旧路径到新路径的映射表

**依赖**
- T53

**验收标准**
- 能支持阶段性迁移
- 不要求一次性替换所有 include