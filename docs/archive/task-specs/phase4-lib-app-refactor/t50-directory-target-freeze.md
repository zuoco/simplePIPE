# T50 冻结目录与目标命名规则（冻结版）

> 任务：T50  
> 日期：2026-04-05  
> 状态：冻结草案（in-progress）

## 1. 适用范围

本文档冻结 Phase 4 的目录根规则与 target 命名规则，作为后续 T51-T77 的前置约束。

## 2. 目录根冻结规则

### 2.1 唯一长期代码根目录

1. `src/lib`：唯一架构层代码根目录。
2. `src/apps`：唯一业务与产品层代码根目录。
3. 除迁移过渡期外，不再新增 `src/foundation`、`src/geometry`、`src/model`、`src/engine`、`src/app`、`src/command`、`src/ui` 这类平级层目录。

### 2.2 目录职责冻结

1. `src/lib` 仅承载跨 app 复用能力（base/platform/runtime/framework）。
2. `src/apps` 仅承载 app 业务闭包（model/engine/workbench/ui/resources/main.cpp）。
3. app 之间禁止直接源代码依赖。

### 2.3 迁移期间允许项

1. 旧目录允许短期保留，但只允许“搬迁”或“兼容别名”改动。
2. 旧目录中禁止新增功能实现（除阻断修复外）。

## 3. Target 命名冻结规则

### 3.1 核心命名

1. 架构统一静态库：`pipecad_lib`
2. 单个 app 业务静态库：`pipecad_app`
3. 单个 app 可执行目标：`pipecad`

### 3.2 多 app 扩展命名

当新增 app `<name>` 时，采用以下固定模式：

1. 业务库：`<name>_app`
2. 可执行：`<name>`
3. 统一链接：`<name>_app` 必须链接 `pipecad_lib`

示例：

1. `offshore_app` + `offshore`
2. `viewer_app` + `viewer`

### 3.3 过渡兼容约束

1. 旧 target（如 `app`、`command`）在过渡期可保留 `ALIAS` 或兼容壳。
2. 新增代码不得继续绑定旧 target 命名。
3. 过渡兼容层在 T75 清理。

## 4. 目录模板冻结（新增 app）

新增 app 必须遵循：

```
src/apps/<name>/
├── CMakeLists.txt
├── main.cpp
├── model/
├── engine/
├── workbench/
├── ui/
└── resources/
```

对应 target：`<name>_app`（静态库） + `<name>`（可执行）。

## 5. 与当前基线关系

当前仓库仍使用旧目录布局与旧目标拓扑，本冻结规则在 M1 开始逐步落地。

1. T53 负责形成新的顶层 CMake 装配拓扑。
2. T56/T57 负责建立新目录骨架。
3. T75 负责清理旧兼容层。
