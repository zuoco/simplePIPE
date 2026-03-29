# 工业软件研发项目上下文

## 项目定位
本项目提供工业软件研发的 Claude Code Skills，覆盖 CAD/CAM/CAE/EDA/仿真/可视化全技术域。

## 可用命令
- `/industrial-dev` — 开源库集成指导与代码研发（OCCT/OSG/VTK/FreeCAD 等）
- `/source-analysis` — 开源库源码阅读与架构分析
- `/architecture-design` — 工业软件架构设计与技术选型
- `/cae-simulation` — CAE/仿真/有限元开发
- `/eda-dev` — EDA/PCB 开发

## 参考文档
`references/` 目录包含 7 份深度参考文档（约 5,700+ 行），按需引用：
- `references/open-source-libraries.md` — 20+ 工业开源库详细介绍与 API 示例
- `references/architecture-patterns.md` — 工业软件架构模式
- `references/development-guide.md` — 开发环境配置与工具指南
- `references/industrial-protocols.md` — 工业协议参考（Modbus/OPC UA/MQTT 等）
- `references/device-connectivity.md` — 设备互联互通方案
- `references/security-compliance.md` — 数据安全与合规指南
- `references/market-analysis.md` — 市场分析框架

## C++ 编码规范
- 使用 OCCT `Handle<T>` 管理几何对象生命周期，禁止裸指针持有 Transient 对象
- OSG 对象使用 `osg::ref_ptr<T>`，禁止裸指针
- 类名 PascalCase，函数名 camelCase，成员变量前缀 m_
- 公开 API 必须有 Doxygen 注释
- 使用 C++17 特性（std::optional, std::variant, structured bindings）
- 几何算法处理数值精度（使用 `Precision::Confusion()` 等 OCCT 精度常量）

## 架构规范
- 业务逻辑层不直接依赖 OCCT/OSG/VTK 具体类型，通过抽象接口访问
- 几何建模、可视化、数据管理三层严格分离
- 插件/模块通过接口注册，支持动态加载
- 大型几何运算放入后台线程，UI 线程保持响应

## 性能规范
- 3D 场景更新使用 OSG NodeVisitor 模式，避免全场景遍历
- OCCT 布尔运算等耗时操作异步执行，提供进度回调
- 网格数据优先使用 VTK Pipeline 模式处理
- 启用 OCCT BVH 空间加速结构用于碰撞检测和拾取

## 安全与稳定性
- OCCT 异常使用 `Standard_Failure` 捕获，不用 `std::exception`
- 文件格式解析（STEP/IGES/STL）必须做错误处理和格式验证
- 多线程访问 OCCT 对象需加锁（OCCT 非线程安全）

## 关键注意事项
- **OCCT Handle 机制**：所有继承自 `Standard_Transient` 的对象必须用 `Handle<T>` 管理
- **OSG ref_ptr**：注意循环引用问题
- **VTK Pipeline**：修改数据后需调用 `Modified()` 触发更新
- **License 合规**：OCCT/FreeCAD/OSG 均为 LGPL，动态链接可免版权但需保留版权声明
- **线程安全**：OCCT 核心库非线程安全，多线程需同步
- **版本兼容**：OCCT 7.x 与 6.x API 差异大，集成前确认版本
