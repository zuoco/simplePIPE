# pipecad resources

本目录是 app 资源骨架位，用于承载：
- QML 入口与组件资源
- 图标与纹理
- 运行时配置模板

当前工程仍使用仓库根目录 ui/main.qml 作为默认入口。
未来新增 app 时，可在对应 app 的 resources 中提供自己的 QML 入口，并在 app CMake 中更新 PIPECAD_QML_MAIN_FILE。
