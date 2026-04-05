# T76 — pipecad app 模板固化

> 任务：T76  
> 日期：2026-04-06  
> 状态：done

## 1. 目标

将现有 `src/apps/pipecad/` 固化为可直接复制的 app 模板，确保未来新增 app 时：
1. 不需要重新设计 `src/lib` 边界。
2. 只需复制目录骨架并修改少量命名与入口配置。
3. 可以复用现有构建拓扑与依赖方向。

## 2. 模板目录

新增 app `<name>` 时，复制并保持以下骨架：

```text
src/apps/<name>/
├── CMakeLists.txt
├── main.cpp
├── model/
├── engine/
├── workbench/
├── ui/
└── resources/
```

其中：
- `model`：业务数据模型（对象定义、属性结构、序列化字段承载）
- `engine`：业务推导与构建逻辑（几何推导、拓扑、校验、重算）
- `workbench`：工作台行为与应用编排边界
- `ui`：QML/Qt 桥接控制层
- `resources`：资源与入口占位（QML、图标、配置）
- `main.cpp`：app 入口与运行时装配

## 3. CMake 模板约束

### 3.1 apps 根入口（src/apps/CMakeLists.txt）

- 使用 `PIPECAD_APP_LIST` 统一注册待构建 app。
- 通过 `pipecad_register_app(app_name)` 校验模板骨架存在性：
  - `model`
  - `engine`
  - `workbench`
  - `ui`
  - `resources`
  - `main.cpp`
- 校验通过后再 `add_subdirectory(${app_name})`。

### 3.2 app 内部入口（src/apps/pipecad/CMakeLists.txt）

固化以下“必改点”变量：
- `PIPECAD_APP_ID`：app 名称（例如 `pipecad`、`viewer`）
- `PIPECAD_QML_MAIN_FILE`：入口 QML 路径

基于 `PIPECAD_APP_ID` 派生目标命名：
- 业务库：`${PIPECAD_APP_ID}_app`
- 可执行：`${PIPECAD_APP_ID}`
- 子域库：`${PIPECAD_APP_ID}_app_model`、`${PIPECAD_APP_ID}_app_engine`、`${PIPECAD_APP_ID}_app_workbench`、`${PIPECAD_APP_ID}_app_ui`、`${PIPECAD_APP_ID}_app_resources`

## 4. 新增 app 最小操作步骤

1. 复制 `src/apps/pipecad` 为 `src/apps/<name>`。
2. 修改新目录下 `CMakeLists.txt`：
   - `PIPECAD_APP_ID` 改为 `<name>`。
   - `PIPECAD_QML_MAIN_FILE` 指向 `<name>` 对应入口。
3. 在 `src/apps/CMakeLists.txt` 的 `PIPECAD_APP_LIST` 增加 `<name>`。
4. 逐步替换各子目录占位实现。
5. 运行构建与测试，确认不破坏现有 app。

## 5. 与 lib 边界关系

- app 仍通过 `pipecad_lib` 访问跨 app 复用能力。
- 不允许跨 app 直接依赖。
- 新增 app 无需修改 `src/lib` 目标拓扑。
