# 管道软件测试案例使用说明

> 本系统采取高覆盖率的模块单元测试和端到端整合测试，通过 Google Test(GTest) 与 Qt Test 等工具框架驱动。所有测试套件被注册于 CMake CTest 系统中。

## 运行全部测试

借助 `pixi` 的环境自动化和构建脚本化设计，测试环境执行极为方便。在根目录运行：

```bash
pixi run test
```
该命令会自动确保工程被编译，并在 `build/debug` 目录下唤落 `ctest --output-on-failure`，依次运行全部分层模块以及集成测试。您可以直接观察终端反馈出的 Pass/Fail 日志与错误位置追溯。

## 模块分层测试列表

本项目为确保系统稳健与分层隔离设计有效，按功能层级分设了 30 余个独立单体测试程序（位于 `tests/` 目录中）。其主要组成部分和验证能力分布如下：

### 1. 核心与工具链基础测试
- `BuildSystem`, `Foundation`: 拦截与验证底层数学转换及系统编译时状态。
- `ProjectSerializer`: 验证工程存取的 JSON 解析逻辑。

### 2. 几何与构件生成测试
- `Geometry`, `MeshStep`, `Model`, `Topology`: 验证管线约束连接逻辑和形状搭建。
- `PipeGeometry`, `ValveFlexBeam`, `ComponentCatalog`, `AccessoryBeam`, `Engine`: 严格验证按照管线特殊规格（如管道口径）驱动参数化衍生实体几何规则的行为。

### 3. 可视化集成测试 (VSG / VTK)
- `Visualization`, `SceneManager`, `CameraFurniture`, `PickHighlight`, `OcctToVtk`, `VtkScene`, `ViewManager`: 围绕 VulkanSceneGraph 及 VTK 视口挂载渲染管线、鼠标交互（拾取、选中框选）的行为进行边界覆盖和压力渲染确认。

### 4. 前端及操作工作台业务测试
- `AppCore`, `WorkbenchBridge`, `QmlModels`, `QmlUiPanels`, `DesignWorkbench`, `SpecWorkbench`: 确保应用各级工作台通过 QML 通讯机制的数据一致性、组件的生命周期（激活被加载，去活不报错）情况有效协同。
- `test_workbench_switch`: QML面板交互动态加载挂载安全策略探测（避免引擎空指针和 OOM 崩溃）。

### 5. 辅助与负荷测试
- `LoadModel`, `LoadCase`: 测试针对结构应力分析而封装的具体载荷及工况。

### 6. 集成联调
- `Integration`: 系统端到端测试，包括加载设计标准、初始化 UI 层和通过几何层重构更新管道。

## 手动运行单项测试用例

在排查特定错误期间，可以使用 `ctest` 的滤网或直接唤起始目标程序来运行特定的场景。

**进入测试环境终端:**
```bash
pixi shell
cd build/debug
```

**运行特定的 CTest:**
```bash
# 只运行名称包含 "Geometry" 的所有测试
ctest -R Geometry --output-on-failure
```

**直接运行独立执行文件:**
```bash
./tests/test_pipe_geometry
```
直接运行能让您更清晰直观地观察到该单元测试崩溃堆栈或是直接结合 gdb 执行单步调试。
