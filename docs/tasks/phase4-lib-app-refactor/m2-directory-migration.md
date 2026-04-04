# M2 目录迁移

## T56 建立 src/lib 目录骨架

**目标**
创建 `src/lib/base`、`src/lib/platform`、`src/lib/runtime`、`src/lib/framework` 基础目录与命名空间规划。

**输入**
- T53 产出

**交付物**
- `src/lib` 目录结构
- 子域入口 `CMakeLists.txt`

**依赖**
- T53

**验收标准**
- `src/lib` 可作为统一架构层容器

## T57 建立 src/apps/pipecad 目录骨架

**目标**
创建当前唯一 app 的业务目录模板。

**输入**
- T53 产出

**交付物**
- `src/apps/CMakeLists.txt`
- `src/apps/pipecad` 目录结构

**依赖**
- T53

**验收标准**
- 未来新增 app 时可直接复用目录模板

## T58 迁移 foundation 到 src/lib/base

**目标**
迁移基础类型、信号、日志、数学工具。

**输入**
- `src/foundation/`
- T56 产出

**交付物**
- 新路径下的基础头文件与源文件
- 兼容头或别名入口

**依赖**
- T56

**验收标准**
- 原有基础测试可继续构建

## T59 迁移 geometry 到 src/lib/platform/occt

**目标**
迁移 `ShapeBuilder`、`BooleanOps`、`StepIO`、`ShapeMesher` 等 OCCT 封装。

**输入**
- `src/geometry/`
- T56 产出

**交付物**
- `src/lib/platform/occt` 下的封装层
- 兼容 include 入口

**依赖**
- T56

**验收标准**
- apps 不再直接耦合 OCCT 头文件

## T60 拆分 visualization 与 vtk-visualization

**目标**
把算法层与 Qt 嵌入层分开，分别进入 lib 和 apps。

**输入**
- `src/visualization/`
- `src/vtk-visualization/`
- T56
- T57

**交付物**
- lib 侧保留纯渲染/转换能力
- apps 侧保留视口与 Qt/QML 集成

**依赖**
- T56
- T57

**验收标准**
- `vtk-visualization` 中算法与 Qt 集成完成分离

## T61 迁移 app 与 command 到 lib

**目标**
迁移 `Document`、`DependencyGraph`、`ProjectSerializer`、`CommandStack`、`CommandRegistry`、`Application`、`WorkbenchManager` 等架构能力。

**输入**
- `src/app/`
- `src/command/`
- T56

**交付物**
- `src/lib/runtime`
- `src/lib/framework`

**依赖**
- T56

**验收标准**
- 架构核心不再留在旧的 `src/app` 与 `src/command`

## T62 迁移 model/engine/ui/main 到 apps

**目标**
把业务模型、业务引擎、UI 与入口收敛到当前 app。

**输入**
- `src/model/`
- `src/engine/`
- `src/ui/`
- `src/main.cpp`
- T57
- T61

**交付物**
- `src/apps/pipecad/model`
- `src/apps/pipecad/engine`
- `src/apps/pipecad/ui`
- `src/apps/pipecad/main.cpp`

**依赖**
- T57
- T61

**验收标准**
- 当前 app 具备完整独立目录闭包