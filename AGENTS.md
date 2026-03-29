# PipeCAD 项目 — AI Agent 工作指令

> **本文件是 AI Agent 的入口指令。每次新会话，AI 应首先阅读此文件。**

---

## 工作流程

当用户说 **"完成任务 TXX"** 或 **"继续下一个任务"** 时，按以下步骤执行：

### Step 1: 从接力文件出发（必读，不得跳过）
```
读取 docs/tasks/current.md
```
- 此文件由上一个 AI 写好，包含：下一个任务 ID、前置依赖状态、需要读取的精确文件列表
- **严禁自行决定读取文件清单**，必须以此文件为唯一入口
- 如果文件指定的任务与用户要求不一致，以用户要求为准，但仍参考文件中的读取指引

### Step 2: 确认任务状态
```
读取 docs/tasks/status.md （仅状态表部分，前 74 行）
```
- 找到目标任务，确认其前置依赖均为 `done`
- **不要读取 status.md 的完成记录索引之后的内容**（完成记录已迁移到 log/ 子目录）
- 如果前置未完成，报告阻塞原因，不要开始

### Step 3: 读取任务详情
```
读取 docs/development-plan.md 中该任务的章节
```
- 获取：交付物列表、接口定义、验收标准

### Step 4: 按需读取前置上下文
```
按 current.md「给 AI 的指令」中列出的文件逐一读取
```
- 只读 current.md 明确列出的日志文件、头文件，不要自行扩展
- 日志文件在 `docs/tasks/log/` 目录，按阶段拆分（见文件索引）
- **直接读取前置任务的 `.h` 头文件**比读日志更准确，优先读头文件

### Step 5: 读取架构参考（如需）
```
读取 docs/architecture.md 中相关章节
```
- 只读与当前任务相关的章节，不要读整个文档

### Step 6: 如涉及领域知识或专门库，读取库指南与 Skills
```
读取 lib/occt/AGENTS.md 或 lib/vsg/AGENTS.md 或 lib/vtk/AGENTS.md
读取 .github/skills/industrial-software-dev/SKILL.md 获取工业软件开发的架构与源码实现指导
```

### Step 7: 实现代码
- 按交付物列表创建/编辑文件
- C++17 标准
- 遵循已有代码风格
- 必须包含单元测试

### Step 8: 编译验证
```bash
pixi run build-debug
pixi run test
```
- 确保编译通过、测试通过

### Step 9: 更新任务状态

**Step 9a**: 更新 `docs/tasks/status.md` 状态表：

1. 将当前任务标记为 `done`，填写完成日期
2. 检查依赖当前任务的后续任务，若所有依赖都 `done`，将其状态从 `pending` 改为 `ready`

**Step 9b**: 追加完成记录到对应的日志文件（文件不存在时新建）：

- Phase 3 任务: `docs/tasks/log/t50-t59.md`（第 50–59 号任务）、`t60-t69.md`……以此类推

使用以下精简格式（**禁止粘贴 C++ 代码块，禁止添加「后续任务注意」字段**）：

```markdown
### TXX — 任务名 (YYYY-MM-DD)

**产出文件**: `A.h` · `A.cpp` · `test_a.cpp`

**接口**: → `src/layer/A.h`, `src/layer/B.h`

**设计决策**:
- 决策1
- 决策2

**已知限制**:
- 限制1（如无则写"无"）
```

**Step 9c**: Git 提交

```bash
git add -A
git commit -m "feat: TXX — 使用中文详细描述实现的功能"
```

- 功能开发统一使用 `feat: TXX — ...` 格式
- Bug 修复使用 `fix: TXX — ...` 格式
- 文档更新使用 `docs: ...` 格式

**Step 9d**: 覆盖重写 `docs/tasks/current.md`，写入下一个任务的信息：

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
| **推荐模型** | Opus / Gemini / Codex / Sonnet（从 status.md 查） |
| **前置依赖** | TYY, TZZ |
| **前置状态** | ✅ 所有依赖已满足 |

## 项目进度

- 已完成: N/总数 个任务
- 当前阶段: Phase X — XXX

## 给 AI 的指令

1. 确认状态: 读取 `docs/tasks/status.md` 状态表（前 74 行）
2. 读取任务详情: `docs/development-plan.md` §TXX
3. 按需读取前置上下文（精确列出，**不要读不相关的内容**）:
   - `docs/tasks/log/tXX-tXX.md` §TXX §TYY  ← 仅当需要了解历史设计决策
   - `src/layer/HeaderA.h`   ← 直接读头文件更准确
4. 如需架构参考: `docs/architecture.md` §相关章节编号
5. 如需库指南: `lib/occt/AGENTS.md` 或 `lib/vsg/AGENTS.md` 或 `lib/vtk/AGENTS.md`
6. 完成后运行 `pixi run build-debug && pixi run test`
7. 验证通过后按 AGENTS.md Step 9 更新 `status.md`、日志文件、本文件
```

如果没有下一个 `ready` 任务（所有任务都完成了），在接力文件中写明当前进度和等待解锁的原因。

### Step 10: 报告完成

向用户报告：
- 完成了什么
- 创建/修改了哪些文件
- 测试是否通过
- Git 提交信息
- 下一个 `ready` 的任务是什么
- **提示用户切换到什么模型**（从接力文件中的推荐模型读取）

---

## 规则

1. **每次任务必须从 `current.md` 出发**，不得自行决定读取文件清单
2. **每次只做一个任务**，除非用户明确要求并行
3. **不要修改不属于当前任务的已有代码**，除非是修 bug
4. **状态文件 (`status.md`) 是唯一的真相来源**，必须保持更新（仅维护状态表，完成记录写 log/ 文件）
5. **完成记录必须足够详细**，让完全无上下文的 AI 也能接续工作（但禁止粘贴 C++ 代码块）
6. **遇到阻塞（编译错误、设计冲突）时**，在状态文件中记录 `blocked` + 原因，不要强行继续
7. **编译必须通过后才能标 `done`**，否则标 `blocked`
8. **任务完成后必须立即更新状态**：每完成一个任务，必须更新 `status.md` 状态表、追加日志文件记录、更新接力文件 `current.md`，**严禁遗漏**
9. **遇到无法修复的故障必须立即停止**：当出现无法自行修复的编译错误、运行时崩溃、环境问题或其他阻塞性故障时，必须：
   - 立即停止当前任务的进一步开发
   - 在 `status.md` 中将任务状态更新为 `blocked`，并在对应日志文件追加故障详情（错误信息、复现步骤、已尝试的修复方案）
   - 向用户报告问题，说明故障原因和建议的解决方向
   - **不得在故障未解决的情况下继续推进或强行标记 `done`**

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
| `docs/tasks/status.md` | **任务状态表（前 74 行）+ 完成记录索引** |
| `docs/tasks/current.md` | **接力文件 — 下一个任务 + 推荐模型（用户看这个切模型）** |
| `docs/tasks/log/t01-t25.md` | Phase 1 完成记录（T01–T25） |
| `docs/tasks/log/t30-t45.md` | Phase 2 完成记录（T30–T45） |
| `lib/occt/AGENTS.md` | OCCT API 使用指南 |
| `lib/vsg/AGENTS.md` | VSG API 使用指南 |
| `lib/vtk/AGENTS.md` | VTK API 使用指南 |
