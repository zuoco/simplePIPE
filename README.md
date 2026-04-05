# 1. 管道系统参数化建模软件 - Pipe Stree Analysis
- [1. 管道系统参数化建模软件 - Pipe Stree Analysis](#1-管道系统参数化建模软件---pipe-stree-analysis)
  - [1.1. 快速开始](#11-快速开始)
    - [1.1.1. 前置条件](#111-前置条件)
    - [1.1.2. 环境初始化](#112-环境初始化)
    - [1.1.3. 编译构建](#113-编译构建)
    - [1.1.4. 运行主程序](#114-运行主程序)
  - [1.2. AI 辅助开发（GitHub Copilot）](#12-ai-辅助开发github-copilot)
    - [1.2.1. 自动注入的编码规范（File Instructions）](#121-自动注入的编码规范file-instructions)
    - [1.2.2. 任务启动 Slash 命令](#122-任务启动-slash-命令)
    - [1.2.3. 领域知识 Skill](#123-领域知识-skill)
  - [1.3. 核心指南文档](#13-核心指南文档)
  - [1.4. License](#14-license)
----------


**项目主入口与核心文档导航**

本项目是基于 OCCT (几何) + VSG (渲染) + QML (UI) 的参数化建模与应力分析协同软件。

## 1.1. 快速开始

### 1.1.1. 前置条件

- **操作系统**: Linux (推荐)
- **工具管理器**: [pixi](https://pixi.sh/latest/)
- **本地预编译库**: `lib/occt`、`lib/vsg` 需手动放置到项目根目录

### 1.1.2. 环境初始化

安装 pixi 后，在项目根目录下安装依赖并首次配置：

```bash
pixi install                # 安装 conda-forge 依赖
pixi run configure-debug    # 首次 CMake Configure (Debug)
```

### 1.1.3. 编译构建

所有构建操作均通过 pixi 执行，确保环境隔离。构建链路为 CMake + Clang + Ninja：

```bash
pixi run configure-debug     # CMake Configure (Debug)
pixi run configure-release   # CMake Configure (Release)
pixi run build-debug         # Configure + 编译 (Debug)
pixi run build-release       # Configure + 编译 (Release)
pixi run test                # 编译 Debug + 运行全部测试
pixi run clean               # 清除 build/ 目录
pixi shell                   # 进入 pixi 交互式环境
```

运行指定测试：

```bash
pixi shell && cd build/debug
ctest -R <TestName> --output-on-failure      # 按名称过滤
./tests/test_<name>                          # 直接运行
```

### 1.1.4. 运行主程序

构建完成后直接运行：

```bash
./build/debug/src/apps/pipecad/pipecad
```

## 1.2. AI 辅助开发（GitHub Copilot）

项目内置 VS Code Copilot 定制化配置，在 `.github/` 目录下提供以下文件：

### 1.2.1. 自动注入的编码规范（File Instructions）

编辑对应类型文件时自动加载，无需手动引用：

| 文件 | 触发范围 | 内容 |
|------|----------|------|
| [.github/instructions/cpp-pipecad.instructions.md](.github/instructions/cpp-pipecad.instructions.md) | `src/**/*.{cpp,h,hpp,cppm}` | Handle/ref_ptr 内存管理、命名规范、精度常量、异常处理、架构边界约束 |
| [.github/instructions/test-conventions.instructions.md](.github/instructions/test-conventions.instructions.md) | `tests/**/*.cpp` | GTest vs Qt6::Test 选择规则、层对应 CMake 链接目标、测试基线要求 |
| [.github/instructions/cmake-layer.instructions.md](.github/instructions/cmake-layer.instructions.md) | `**/CMakeLists.txt` | 新旧目录构建边界、依赖链方向、目标别名映射 |

### 1.2.2. 任务启动 Slash 命令

在聊天框输入 `/start-task` 选择 prompt，填入任务编号（如 `T78`），即可触发完整任务启动流程：

1. 读取 `docs/tasks/current.md` 获取上下文
2. 检查 `docs/tasks/status.md` 确认前置依赖
3. 读取任务详情卡 → 实现代码 → 编译验证
4. 收口：更新 status、追加日志、Git 提交、重写 current.md

文件位置：[.github/prompts/start-task.prompt.md](.github/prompts/start-task.prompt.md)

### 1.2.3. 领域知识 Skill

输入 `/industrial-software-dev` 或在对话中描述相关需求时自动匹配，提供 OCCT/VSG/VTK 深度集成指导：

文件位置：[.github/skills/industrial-software-dev/SKILL.md](.github/skills/industrial-software-dev/SKILL.md)

## 1.3. 核心指南文档

- [文档导航](docs/README.md) — 文档目录结构与推荐阅读顺序
- [架构设计说明](docs/architecture.md) — 8 层工程分层、命令模式编辑链路与业务引擎流转体系
- [AI 工作准则](AGENTS.md) — 任务接力流程、构建/测试约束和完整编码规范
- [任务状态机](docs/tasks/status.md) — 项目阶段完成情况与历史记录索引

## 1.4. License

本项目基于 [Apache License 2.0](LICENSE) 开源协议发布。

Copyright 2024-2026 PipeCAD Contributors

## License

本项目基于 [Apache License 2.0](LICENSE) 开源协议发布。

Copyright 2024-2026 PipeCAD Contributors

---

> 项目共完成 4 个阶段（Phase 1–4），当前处于架构稳定期。详见 [任务状态机](docs/tasks/status.md)。