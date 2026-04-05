# M0 规则冻结

## T50 冻结目录与目标命名规则

**目标**
确定 `src/lib` 与 `src/apps` 为唯一长期代码根目录，并冻结 target 命名规则。

**输入**
- `docs/archive/task-specs/phase4-lib-app-refactor-plan.md`
- `docs/architecture.md`
- `src/CMakeLists.txt`

**改动范围**
- `docs/`
- 顶层 CMake 设计说明

**交付物**
- 固定的目录规则说明
- 固定的 target 命名规则
- `pipecad_lib` / `pipecad_app` / `pipecad` 的命名约定
- 冻结文档：`docs/archive/task-specs/phase4-lib-app-refactor/t50-directory-target-freeze.md`

**依赖**
- 无

**验收标准**
- 明确架构层唯一根目录为 `src/lib`
- 明确业务层唯一根目录为 `src/apps`
- 明确未来新增 app 的目录模板和命名方式

**当前进展（2026-04-05）**
- 已创建规则冻结文档草案：`docs/archive/task-specs/phase4-lib-app-refactor/t50-directory-target-freeze.md`
- 已冻结 `pipecad_lib` / `pipecad_app` / `pipecad` 与 `<name>_app` / `<name>` 扩展命名模式
- 待在 T53（构建重编组）中将规则映射到新的顶层 CMake 拓扑

## T51 冻结 include/import 规则

**目标**
定义 lib 与 apps 的允许依赖路径、模块导入规则和禁止路径。

**输入**
- T50 产出
- `docs/archive/task-specs/phase4-lib-app-refactor-plan.md`

**改动范围**
- `docs/`
- 后续公共头命名规范

**交付物**
- include/import 白名单规则
- 禁止跨域规则
- 第三方库访问约束

**依赖**
- T50

**验收标准**
- apps 不允许直接 include `OCCT`、`VSG`、`VTK` 头
- `lib::runtime` 不允许反向依赖 `lib::framework`
- 不允许跨 app 直接依赖

**当前进展（2026-04-05）**
- 已创建冻结文档：`docs/archive/task-specs/phase4-lib-app-refactor/t51-include-import-freeze.md`
- 已冻结 lib/apps 依赖白名单与黑名单规则（含 include/import/CMake 依赖三类约束）
- 已明确 apps 通过 `lib` 接口访问第三方能力，禁止直接 include OCCT/VSG/VTK
- 已冻结 `lib::runtime` 禁止反向依赖 `lib::framework` 与跨 app 直接依赖规则

## T52 冻结线程安全边界

**目标**
明确主线程对象、后台线程对象、快照边界与结果回投边界。

**输入**
- T50 产出
- `docs/archive/task-specs/phase4-lib-app-refactor-plan.md`

**改动范围**
- `docs/`
- 并发设计说明

**交付物**
- 主线程专属对象清单
- 后台线程允许行为清单
- 快照与只读接口约束

**依赖**
- T50

**验收标准**
- `CommandStack`、`Document` 写接口、`SceneManager` 写操作的主线程所有权明确
- 后台线程只允许消费快照和只读接口

**当前进展（2026-04-05）**
- 已创建冻结文档：`docs/archive/task-specs/phase4-lib-app-refactor/t52-thread-boundary-freeze.md`
- 已冻结主线程专属对象清单：CommandStack、Document 写接口、DependencyGraph 写接口、RecomputeEngine、SceneManager 写操作、SelectionManager 写接口、WorkbenchManager、所有 Qt/QML 对象
- 已冻结后台线程允许行为：只读快照消费、const 方法调用、独立几何推导计算、通过 Qt::QueuedConnection 发射结果信号
- 已冻结快照边界：包含版本令牌、PipePoint 值拷贝、PipeSpec 参数、依赖拓扑只读视图
- 已冻结结果回投协议：唯一合法通道为 Qt::QueuedConnection，回投时校验版本令牌，版本不匹配则静默丢弃
- 已明确迁移期约束：T68 之前保持单线程串行执行，T68-T71 搭建并发框架，M4 后方可启用后台重算