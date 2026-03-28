# PipeCAD 项目 — AI Agent 工作指令

> **本文件是 AI Agent 的入口指令。每次新会话，AI 应首先阅读此文件。**

---

## 工作流程

当用户说 **"完成任务 TXX"** 或 **"继续下一个任务"** 时，按以下步骤执行：

### Step 1: 读取任务状态
```
读取 docs/tasks/status.md
```
- 找到用户指定的任务（或第一个 `ready` 状态的任务）
- 确认其前置依赖均为 `done`
- 如果前置未完成，报告阻塞原因，不要开始

### Step 2: 读取任务详情
```
读取 docs/development-plan.md 中该任务的章节
```
- 获取：交付物列表、接口定义、验收标准

### Step 3: 读取前置上下文
```
读取 docs/tasks/status.md 的「完成记录」段落
```
- 找到前置任务的完成记录，了解已有接口和关键决策
- 读取前置任务产出的头文件（`.h`），作为接口依赖

### Step 4: 读取架构参考
```
读取 docs/architecture.md 中相关章节
```
- 只读与当前任务相关的章节，不要读整个文档

### Step 5: 如涉及领域知识或专门库，读取库指南与 Skills
```
读取 lib/occt/AGENTS.md 或 lib/vsg/AGENTS.md 或 lib/vtk/AGENTS.md
读取 .github/skills/industrial-software-dev/SKILL.md 获取工业软件开发的架构与源码实现指导
```

### Step 6: 实现代码
- 按交付物列表创建/编辑文件
- C++17 标准
- 遵循已有代码风格
- 必须包含单元测试

### Step 7: 编译验证
```bash
pixi run build-debug
pixi run test
```
- 确保编译通过、测试通过

### Step 8: 更新任务状态

在 `docs/tasks/status.md` 中：

1. **更新状态表**: 将当前任务标记为 `done`，填写完成日期
2. **更新后续任务状态**: 检查依赖当前任务的后续任务，如果其所有依赖都 `done`，将其状态从 `pending` 改为 `ready`
3. **追加完成记录**: 在 `COMPLETION LOG` 区域追加：

```markdown
### TXX — 任务名 (YYYY-MM-DD)

**产出文件**:
- src/xxx/Yyy.h
- src/xxx/Yyy.cpp
- tests/test_yyy.cpp

**关键接口** (后续任务需要知道的):
```cpp
// 粘贴核心 class/function 签名
```

**设计决策**:
- 列出实现中做出的重要决策

**已知限制**:
- 列出遗留问题或待优化项

**后续任务注意**:
- 对下游任务的特别提示
```

### Step 8b: Git 提交

```bash
git add -A
git commit -m "feat: TXX — 使用中文详细描述实现的功能"
```

- 功能开发统一使用 `feat: TXX — ...` 格式
- Bug 修复使用 `fix: TXX — ...` 格式
- 文档更新使用 `docs: ...` 格式

### Step 8c: 更新接力文件

**覆盖重写** `docs/tasks/current.md`，写入下一个任务的信息：

```markdown
# 当前任务状态

> **此文件每次任务完成后由 AI 覆盖重写。**
> **下一个 AI 会话只需读取此文件即可开始工作。**

---

## 下一个任务

| 属性 | 值 |
|------|---|
| **任务 ID** | TXX |
| **任务名** | XXX |
| **推荐模型** | Opus / Gemini / Codex / Sonnet (从status.md查) |
| **前置依赖** | TYY, TZZ |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: N/22 个任务
- 当前阶段: Phase X — XXX

## 上一个完成的任务

TXX — 任务名 (日期)
- 产出: [关键文件列表]
- 关键接口: [核心签名摘要]
- 注意事项: [给下一个任务的提示]

## 给 AI 的指令

1. 读取 `docs/development-plan.md` 中 **TXX** 章节
2. 读取 `docs/architecture.md` **§X** 相关章节
3. 读取前置代码: [列出需要读的.h文件路径]
4. 如需领域知识或库指南: 读取 `.github/skills/industrial-software-dev/SKILL.md` 和 `lib/xxx/AGENTS.md`
5. 完成后运行 `pixi run build-debug && pixi run test`
6. 验证通过后更新 `docs/tasks/status.md` 和本文件
```

如果没有下一个 `ready` 任务（所有 `ready` 任务都完成了），在接力文件中写明"等待其他任务完成后解锁"。

### Step 9: 报告完成
向用户报告：
- 完成了什么
- 创建/修改了哪些文件
- 测试是否通过
- Git 提交信息
- 下一个 `ready` 的任务是什么
- **提示用户切换到什么模型**（从接力文件中的推荐模型读取）

---

## 规则

1. **每次只做一个任务**，除非用户明确要求并行
2. **不要修改不属于当前任务的已有代码**，除非是修 bug
3. **状态文件 (`status.md`) 是唯一的真相来源**，必须保持更新
4. **完成记录必须足够详细**，让完全无上下文的 AI 也能接续工作
5. **遇到阻塞（编译错误、设计冲突）时**，在状态文件中记录 `blocked` + 原因，不要强行继续
6. **编译必须通过后才能标 `done`**，否则标 `blocked`

---

## 项目技术栈

| 技术 | 版本 | 位置 |
|------|------|------|
| OCCT | 8.0.0 | lib/occt/ |
| VSG | 1.1.13 | lib/vsg/ |
| VTK | 9.6.0 | lib/vtk/ (未来) |
| Qt6 Quick | ≥6.5 | pixi (conda-forge) |
| nlohmann/json | * | pixi (conda-forge) |
| GTest | * | pixi (conda-forge) |
| C++ | 17 | — |
| 构建 | pixi + CMake + Ninja | — |

---

## 文件索引

| 文件 | 用途 |
|------|------|
| `docs/architecture.md` | 架构设计（数据模型、分层、UI设计） |
| `docs/development-plan.md` | 任务详情（交付物、验收标准） |
| `docs/tasks/status.md` | **任务状态 + 完成记录（状态机核心）** |
| `docs/tasks/current.md` | **接力文件 — 下一个任务 + 推荐模型（用户看这个切模型）** |
| `lib/occt/AGENTS.md` | OCCT API 使用指南 |
| `lib/vsg/AGENTS.md` | VSG API 使用指南 |
| `lib/vtk/AGENTS.md` | VTK API 使用指南 |
