# T51 冻结 include/import 规则（冻结版）

> 任务：T51  
> 日期：2026-04-05  
> 状态：冻结完成（done）

## 1. 适用范围

本文档冻结 Phase 4 的 include/import 依赖边界规则，作为 T53-T77 的前置约束。

规则适用于：
1. 头文件包含（`#include`）
2. 模块导入（`import`，在 C++20 Modules 落地后同样受本规则约束）
3. CMake 目标依赖（`target_link_libraries`）

## 2. 依赖方向总则

依赖方向冻结为：

```
lib::base -> lib::platform -> lib::runtime -> lib::framework -> apps/<name>
```

补充约束：
1. `apps/<name>` 仅允许依赖统一架构库对外接口（未来为 `pipecad_lib`）。
2. 任何上层不得反向依赖下层调用方。
3. 同层不同 app 之间禁止直接源代码依赖。

## 3. 白名单规则（允许）

### 3.1 lib 域

1. `lib::base`
   - 允许：C++ 标准库、基础第三方（如日志/测试辅助，按后续实际落地）
   - 禁止：依赖 `lib::runtime`、`lib::framework`、任意 `apps/*`

2. `lib::platform`
   - 允许：`lib::base`、第三方平台库（OCCT/VSG/VTK）
   - 职责：封装第三方细节并通过 facade/公开接口向上暴露

3. `lib::runtime`
   - 允许：`lib::base`、`lib::platform`
   - 禁止：反向依赖 `lib::framework`

4. `lib::framework`
   - 允许：`lib::base`、`lib::platform`、`lib::runtime`
   - 禁止：依赖任意 `apps/*`

### 3.2 apps 域

1. `apps/<name>`
   - 允许：
     - `pipecad_lib`（或过渡期中指向统一架构能力的兼容 target）
     - `apps/<name>` 自身子目录（`model`、`engine`、`workbench`、`ui`、`resources`）
   - 禁止：
     - 直接依赖其他 app（如 `apps/other_app/*`）
     - 直接链接其他 app 的业务库目标（如 `<other>_app`）

## 4. 黑名单规则（禁止）

### 4.1 apps 禁止直接 include/import 第三方平台头

`apps/*` 中禁止直接包含或导入以下头/模块：
1. OCCT（如 `TopoDS_*`、`BRep*`、`gp_*`）
2. VSG（如 `vsg/*`）
3. VTK（如 `vtk*`）

必须经由 `lib::platform` 或 `pipecad_lib` 对外接口访问能力。

### 4.2 runtime 反向依赖禁止

`lib::runtime` 禁止以任何方式依赖 `lib::framework`，包括但不限于：
1. include/import `framework` 头或模块
2. CMake `target_link_libraries(runtime ... framework ... )`
3. 通过跨层“工具头”隐式引入 framework 类型

### 4.3 跨 app 依赖禁止

禁止任意 app 对另一个 app 产生编译期或链接期依赖，包括：
1. `#include "src/apps/<other>/..."`
2. 链接 `<other>_app`
3. 复用其他 app 的私有 UI/工作台实现

如需复用，必须先下沉到 `src/lib` 公共能力域。

## 5. 第三方库访问约束

1. OCCT/VSG/VTK 访问入口统一收敛到 `lib::platform`。
2. apps 只能消费经 `lib` 暴露的稳定接口，不得感知第三方头路径。
3. 第三方升级或替换影响应限定在 `lib::platform` 内部，不向 apps 扩散。

## 6. 迁移期兼容约束

考虑当前代码仍在旧目录拓扑，迁移期允许以下临时状态：
1. 旧目录中的历史 include 先保留，按 T55 兼容层策略分步迁移。
2. 与本规则冲突的历史引用可暂留，但禁止新增同类违规引用。
3. 进入 T53（构建重编组）后，新增目标与新代码必须优先遵循本规则。

## 7. 与后续任务关系

1. T53：将本规则映射为顶层 CMake 拓扑和目标依赖图。
2. T55：提供旧 include 到新路径的兼容映射与过渡策略。
3. T59/T60/T61/T62：在物理迁移中逐步消除历史违规引用。
4. T75：清理兼容层并将本规则转为唯一生效规则。