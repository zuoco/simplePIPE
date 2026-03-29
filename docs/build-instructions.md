# 管道软件构建编译说明

> 本文档描述了 `qml-vsg-occt` 软件项目的环境配置与构建流程。项目采用基于 `pixi` 的包环境隔离与工具链组合，搭配 CMake/Ninja 实现快速增量构建。

## 环境要求

- **操作系统**: Linux (推荐) / Windows
- **编译器**: GCC (Linux) / MSVC (Windows，需支持 C++17 或以上)
- **工具管理器**: [pixi](https://pixi.sh/latest/) (用于依赖获取和环境隔离)
- **依赖库 (早期版通过源码预置及pixi安装)**:
  - CMake >= 3.24
  - Qt6 >= 6.5 (需组件 Quick, Qml, Test)
  - OpenCASCADE >= 8.0.0 (OCCT，项目早期已内置于 `lib/occt`)
  - VulkanSceneGraph >= 1.1.13 (VSG，项目早期已内置于 `lib/vsg`)
  - VTK >= 9.6 (用于应力结果分析，开发期将迁入)
  - nlohmann_json

## 快速开始

本项目依赖 `pixi` 完成开发环境管理，不再需要手工污染系统环境全局安装依赖，`pixi` 的 `Conda/Forge` 生态将自动按需下载所有必备包和头文件。

### 1. 安装 pixi
如果您还没有安装 pixi，请直接通过官方脚本安装：
```bash
curl -fsSL https://pixi.sh/install.sh | bash
```

### 2. 获取代码
```bash
git clone <your-repository-url>
cd qml-vsg-occt
```

### 3. 构建软件
在包含 `pixi.toml` 的项目根目录下直接使用 pixi 即可：

```bash
# 自动配置并编译 Debug 版本 (依赖于 `configure-debug` task)
pixi run build-debug
```

若需 Release 构建，也可执行：
```bash
pixi run build-release
```
此时系统会通过 CMake 调用 Ninja 并针对各分层进行构建和链接，生成可执行文件及各层静态库到对应的 `build/debug` 或 `build/release` 目录下。

### 4. 运行主程序
如果正常构建通过，直接运行主程序（假设是在 Debug 模式下）：
```bash
./build/debug/src/pipecad_app
```
*注：主程序的 QML 加载配置已被写入编译定义中(以绝对路径绑定)*

## 构建层级结构

由于采用了严格依赖分化结构，构建包含以下部分（详见 `src/CMakeLists.txt`）：
- **基础及功能层** (静态库): `foundation`, `geometry`, `model`, `engine`, `visualization`, `app`, `ui`
- **应用层** (可执行): `pipecad_app` (主 Qt6 / Qml 程序入口)

## 开发者提示

进入与项目隔离好的 Shell 进行测试或者独立命令敲击，可执行：
```bash
pixi shell
```
该 Shell 已预置了全部环境变量和 CMAKE_PREFIX 路径，可以直接手动运行常规的 CMake 或 CTest 操作。
