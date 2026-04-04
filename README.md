# 管道系统参数化建模软件

**项目主入口与核心文档导航**

本项目是基于 OCCT (几何) + VSG (渲染) + QML (UI) 的参数化建模与应力分析协同软件。

## 快速开始

### 前置条件

- **操作系统**: Linux (推荐)
- **工具管理器**: [pixi](https://pixi.sh/latest/)（构建脚本会自动安装）
- **本地预编译库**: `lib/occt`、`lib/vsg`、`lib/vtk` 需手动放置到项目根目录

### 一键初始化

在项目根目录下运行环境初始化脚本，自动完成 pixi 安装、依赖下载、工具链检查和首次 CMake Configure：

```bash
bash scripts/setup.sh
```

验证当前环境状态：

```bash
bash scripts/setup.sh --verify
```

### 编译构建

所有构建操作均通过 pixi 执行，确保环境隔离。在项目根目录下运行：

```bash
bash scripts/build.sh                # 默认 Debug 构建
bash scripts/build.sh release        # Release 构建
bash scripts/build.sh test           # Debug 构建 + 运行全部测试
bash scripts/build.sh test -R Engine # 仅运行 Engine 相关测试
bash scripts/build.sh run            # Debug 构建并运行主程序
bash scripts/build.sh full           # clean + build + test 全量构建
bash scripts/build.sh clean          # 清除构建产物
bash scripts/build.sh status         # 查看构建状态
bash scripts/build.sh help           # 显示完整帮助
```

### 等价 pixi 命令

如果不使用构建脚本，也可以直接使用 pixi 任务命令：

```bash
pixi run configure-debug     # CMake Configure (Debug)
pixi run configure-release   # CMake Configure (Release)
pixi run build-debug         # Configure + 编译 (Debug)
pixi run build-release       # Configure + 编译 (Release)
pixi run test                # 编译 Debug + 运行全部测试
pixi run clean               # 清除 build/ 目录
pixi shell                   # 进入 pixi 交互式环境
```

### 运行主程序

构建完成后直接运行：

```bash
./build/debug/src/pipecad_app
```

## 核心指南文档

- [架构设计说明](docs/architecture.md) — 了解当前 8 层工程分层、命令模式编辑链路与业务引擎流转体系
- [AI 准则](AGENTS.md) — 了解当前任务接力流程、构建/测试约束和文档入口
- [任务状态机(status)](docs/tasks/status.md) — 查看项目阶段完成情况与历史记录索引

## License

本项目基于 [Apache License 2.0](LICENSE) 开源协议发布。

Copyright 2024-2026 PipeCAD Contributors

---

> 附相关工作维护状态，详见：
> [任务状态机(status)](docs/tasks/status.md) 以及 [AI 准则(AGENTS)](AGENTS.md)。