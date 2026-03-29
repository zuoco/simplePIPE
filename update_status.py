import re

with open("docs/tasks/status.md", "r") as f:
    content = f.read()

# Update T42 status to done
content = re.sub(r'\| T42 \| VTK-QML 桥接 \| `ready` \|(.*?)\|', r'| T42 | VTK-QML 桥接 | `done` |\1| 2026-03-29 |', content)
# Update T44 status to ready (since T42 and T39 and T33 are done. Wait, T33 is done because T43 says "T33(LoadCase/Combo) — 已完成"). Wait, T44 dependencies: T33, T39, T42. So T44 becomes ready!
content = re.sub(r'\| T44 \| AnalysisWorkbench 工作台 \| `pending` \|(.*?)\|', r'| T44 | AnalysisWorkbench 工作台 | `ready` |\1|', content)

# Append completion log
log = """### T42 — VTK-QML 桥接 (2026-03-29)

**产出文件**:
- `src/vtk-visualization/VtkViewport.h/cpp`
- `ui/panels/VtkViewport.qml`
- `tests/test_vtk_qml.cpp`

**关键接口**:
```cpp
class VtkViewport : public QQuickFramebufferObject {
    void setSceneManager(VtkSceneManager* manager);
    vtkGenericRenderWindowInteractor* interactor() const;
    vtkGenericOpenGLRenderWindow* renderWindow() const;
};
```

**设计决策**:
- 采用 `QQuickFramebufferObject` 并实现 `VtkFboRenderer`，将 `vtkGenericOpenGLRenderWindow` 渲染结果挂载到 Qt Quick 组合图中。
- 移除了 `VtkViewport` 构造函数中的 `interactor_->Initialize()`，避免因上下文中未生效 OpenGL 产生 SIGSEGV 崩溃。
- `ViewManager` 接口添加 `setVtkComponents`。

**后续任务注意**:
- `AnalysisWorkbench` 工作台激活时，前端 `main.qml` 会注入 `VtkViewport` 对象而非 `VsgViewport` 供 `ViewManager` 使用。
- 如果进行快照截取等离屏渲染交互，需要注意 QML 的 FBO 管理周期。
"""

content = content.replace("<!-- === COMPLETION LOG END === -->", log + "\n<!-- === COMPLETION LOG END === -->")

with open("docs/tasks/status.md", "w") as f:
    f.write(content)
