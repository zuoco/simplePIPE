# M0 规则冻结

## T50 冻结目录与目标命名规则

**目标**
确定 `src/lib` 与 `src/apps` 为唯一长期代码根目录，并冻结 target 命名规则。

**输入**
- `docs/lib-app-refactor-plan.md`
- `docs/architecture.md`
- `src/CMakeLists.txt`

**改动范围**
- `docs/`
- 顶层 CMake 设计说明

**交付物**
- 固定的目录规则说明
- 固定的 target 命名规则
- `pipecad_lib` / `pipecad_app` / `pipecad` 的命名约定

**依赖**
- 无

**验收标准**
- 明确架构层唯一根目录为 `src/lib`
- 明确业务层唯一根目录为 `src/apps`
- 明确未来新增 app 的目录模板和命名方式

## T51 冻结 include/import 规则

**目标**
定义 lib 与 apps 的允许依赖路径、模块导入规则和禁止路径。

**输入**
- T50 产出
- `docs/lib-app-refactor-plan.md`

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

## T52 冻结线程安全边界

**目标**
明确主线程对象、后台线程对象、快照边界与结果回投边界。

**输入**
- T50 产出
- `docs/lib-app-refactor-plan.md`

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