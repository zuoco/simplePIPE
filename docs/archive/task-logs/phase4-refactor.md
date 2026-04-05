# Phase 4 完成记录 — lib/apps 架构重构 (T50–T77)

> **实施计划书**: `docs/lib-app-refactor-plan.md`
> **任务卡目录**: `docs/tasks/phase4-lib-app-refactor/`
> **任务状态**: `docs/tasks/status.md`

---

### T50 — 冻结目录与目标命名规则 (2026-04-05)

**产出文件**: `docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md` · `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md` · `docs/lib-app-refactor-plan.md` · `src/CMakeLists.txt`

**接口**: → `docs/tasks/phase4-lib-app-refactor/t50-directory-target-freeze.md`

**设计决策**:
- 确立 `src/lib` 与 `src/apps` 为唯一长期代码根目录
- 架构统一静态库固定命名 `pipecad_lib`
- 当前 app 业务库固定命名 `pipecad_app`，可执行固定命名 `pipecad`
- 新增 app 采用 `<name>_app`（业务库）+ `<name>`（可执行）命名对
- 新增 app 目录模板固定为 `src/apps/<name>/model|engine|workbench|ui|resources|main.cpp`
- 旧 target（`app`、`command`）过渡期保留 ALIAS 壳，在 T75 清理

**已知限制**:
- 本任务仅冻结规则文档，代码目录结构与构建拓扑仍在旧结构，需等 T53–T62 逐步落地

### T51 — 冻结 include/import 规则 (2026-04-05)

**产出文件**: `docs/tasks/phase4-lib-app-refactor/t51-include-import-freeze.md` · `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md` · `docs/lib-app-refactor-plan.md`

**接口**: → `docs/tasks/phase4-lib-app-refactor/t51-include-import-freeze.md`

**设计决策**:
- 冻结 include/import/CMake 依赖三类统一约束，规则同时覆盖 `#include`、`import`、`target_link_libraries`
- 冻结分层依赖方向：`lib::base -> lib::platform -> lib::runtime -> lib::framework -> apps/<name>`
- 明确 apps 禁止直接 include/import OCCT/VSG/VTK，第三方访问统一经 `lib::platform` 收口
- 明确 `lib::runtime` 禁止反向依赖 `lib::framework`（编译期与链接期均禁止）
- 明确禁止跨 app 直接依赖，跨 app 复用必须先下沉到 `src/lib`

**已知限制**:
- 当前仓库仍处于旧目录拓扑，历史违规 include 需在 T53/T55/T59-T62 迁移阶段逐步消除

### T52 — 冻结线程安全边界 (2026-04-05)

**产出文件**: `docs/tasks/phase4-lib-app-refactor/t52-thread-boundary-freeze.md` · `docs/tasks/phase4-lib-app-refactor/m0-rule-freeze.md`

**接口**: → `docs/tasks/phase4-lib-app-refactor/t52-thread-boundary-freeze.md`

**设计决策**:
- 冻结单一可变状态线程原则：所有写操作（Document/CommandStack/SceneManager/Qt对象）主线程独占
- 冻结主线程专属对象清单：CommandStack、Document 写接口、DependencyGraph 写接口、RecomputeEngine、SceneManager 写操作、SelectionManager 写接口、WorkbenchManager、所有 Qt/QML/VTK 对象
- 冻结后台线程允许行为：只读快照消费、const 方法、独立 OCCT 几何推导、Qt::QueuedConnection 信号发射
- 冻结快照边界组成：版本令牌（用于失效检测）+ PipePoint/PipeSpec 值拷贝 + 依赖拓扑只读视图
- 冻结结果回投协议：唯一合法通道为 Qt::QueuedConnection；回投前校验版本令牌，不匹配则静默丢弃
- 明确失效丢弃双重检查：版本号校验 + 对应 UUID 对象是否仍存在
- 明确迁移期约束：T68 前保持全串行，T68-T71 搭建并发框架，M4 后方可启用后台几何重算

**已知限制**:
- 当前系统全程单线程执行，本文档的并发规则在 T68-T71 才实际落地；本任务仅冻结设计约束
