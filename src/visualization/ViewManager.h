#pragma once

#include "CameraController.h"
#include "SceneFurniture.h"
#include "SceneManager.h"

#include <gp_Pnt.hxx>

#include <map>
#include <string>

namespace vtk_vis { class VtkSceneManager; }

namespace visualization {

/// @brief 视图管理器 — Layer 5 统一门面
///
/// 协调 VSG 与 VTK 双视口的渲染与交互。上层 QML/工具栏只与 ViewManager 交互，
/// 无需感知底层渲染引擎。
///
/// 核心职责：
///   - 视口路由：VSG ↔ VTK 切换
///   - 相机控制转发：fitAll / setViewPreset
///   - 视图状态缓存：工作台切换时保存/恢复相机状态
///   - 渲染模式管理：Solid / Wireframe / SolidWithEdges / Beam
///   - 可见性控制：按 Category 开关节点显隐
///   - 场景装饰控制：网格/坐标轴/标注
///   - LOD 层级管理
///   - 截图（预留）
class ViewManager {
public:
    /// 活跃视口类型
    enum class ActiveViewport { VSG, VTK };

    /// 渲染模式
    enum class RenderMode { Solid, Wireframe, SolidWithEdges, Beam };

    /// 可见性分类
    enum class Category {
        PipePoints, Segments, Accessories, Supports, Beams,
        Annotations, LoadArrows, StressContour
    };

    /// LOD 精度档位
    enum class LodLevel { Draft, Normal, Fine };

    /// 相机状态快照（用于工作台切换时缓存/恢复）
    struct CameraState {
        vsg::dvec3 eye    {0, -10, 0};
        vsg::dvec3 center {0, 0, 0};
        vsg::dvec3 up     {0, 0, 1};
    };

    ViewManager();

    /// 注入 VSG 子系统（SceneManager + CameraController + SceneFurniture）
    void setVsgComponents(SceneManager* scene, CameraController* camera, SceneFurniture* furniture);
    void setVtkComponents(vtk_vis::VtkSceneManager* vtkScene);

    // === 视口路由 ===
    void setActiveViewport(ActiveViewport vp);
    ActiveViewport activeViewport() const;

    // === 相机控制（转发给当前活跃视口） ===
    void fitAll();
    void setViewPreset(ViewPreset preset);
    void saveViewState(const std::string& workbenchId);
    void restoreViewState(const std::string& workbenchId);

    // === 渲染模式 ===
    void setRenderMode(RenderMode mode);
    RenderMode renderMode() const;

    // === 可见性控制 ===
    void setCategoryVisible(Category cat, bool visible);
    bool isCategoryVisible(Category cat) const;

    // === 场景装饰 ===
    void setGridVisible(bool visible);
    bool isGridVisible() const;
    void setTriadVisible(bool visible);
    bool isTriadVisible() const;

    // === LOD ===
    void setLodLevel(LodLevel level);
    LodLevel lodLevel() const;

    // === StatusBar 数据 ===
    void setMouseWorldPos(const gp_Pnt& pos);
    gp_Pnt currentMouseWorldPos() const;

    // === 截图（预留） ===
    bool captureImage(const std::string& path);

private:
    ActiveViewport activeVp_ = ActiveViewport::VSG;
    RenderMode     renderMode_ = RenderMode::Solid;
    LodLevel       lodLevel_ = LodLevel::Normal;
    bool           triadVisible_ = true;

    std::map<Category, bool> visibility_;
    std::map<std::string, CameraState> viewStateCache_;

    gp_Pnt mouseWorldPos_{0.0, 0.0, 0.0};

    // 非拥有指针（VSG 子系统）
    SceneManager*     vsgScene_   = nullptr;
    CameraController* vsgCamera_  = nullptr;
    SceneFurniture*   vsgFurni_   = nullptr;

    // VTK 子系统指针预留（T38/T42 时注入）
    vtk_vis::VtkSceneManager* vtkScene_ = nullptr;
};

} // namespace visualization
