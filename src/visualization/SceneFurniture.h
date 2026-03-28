#pragma once

#include <vsg/nodes/Group.h>
#include <vsg/nodes/Switch.h>

#include <cstdint>

namespace visualization {

/// @brief 场景基础设施节点
///
/// 提供 3D 视口中的辅助可视化元素：
///   - 坐标轴指示器（X=红, Y=绿, Z=蓝，三条轴线）
///   - 地面网格（XY 平面，N×N 格，可开关显示）
///   - 渐变背景（通过 clearColor 方式设置，返回颜色值供外部使用）
///
/// 所有几何均使用 VSG VertexDraw 节点存储顶点数据。
/// 注意：渲染前需要在外部 compile 场景图以绑定 Vulkan 资源。
///
/// 典型用法：
/// @code
///   SceneFurniture furniture;
///   sceneRoot->addChild(furniture.axisNode());
///   sceneRoot->addChild(furniture.gridSwitch()); // 注意：gridSwitch 含 Switch 节点
///   furniture.setGridVisible(false);             // 隐藏地面网格
/// @endcode
class SceneFurniture {
public:
    /// @param axisLength   坐标轴线段长度（mm）
    /// @param gridSize     网格总尺寸（mm，单边）
    /// @param gridDivisons 网格分格数
    explicit SceneFurniture(
        double   axisLength   = 200.0,
        double   gridSize     = 5000.0,
        uint32_t gridDivisons = 20);

    /// 坐标轴节点（三轴彩色线段 Group，每轴 2 个顶点）
    vsg::ref_ptr<vsg::Group> axisNode() const;

    /// 地面网格内容节点（XY 平面线网格 Group，含所有顶点）
    vsg::ref_ptr<vsg::Group> gridNode() const;

    /// 地面网格的 Switch 包装节点（应将此节点添加到场景，而非 gridNode）
    /// 通过 setGridVisible() 控制其可见性
    vsg::ref_ptr<vsg::Switch> gridSwitch() const;

    /// 控制地面网格是否显示
    void setGridVisible(bool visible);

    /// 查询地面网格当前可见性
    bool isGridVisible() const;

    /// 背景顶部颜色（#E8E8E8）
    static constexpr float backgroundTopR()   { return 0.910f; }
    static constexpr float backgroundTopG()   { return 0.910f; }
    static constexpr float backgroundTopB()   { return 0.910f; }

    /// 背景底部颜色（#D0D0D0）
    static constexpr float backgroundBotR()   { return 0.816f; }
    static constexpr float backgroundBotG()   { return 0.816f; }
    static constexpr float backgroundBotB()   { return 0.816f; }

private:
    vsg::ref_ptr<vsg::Group>  buildAxisNode(double axisLength) const;
    vsg::ref_ptr<vsg::Group>  buildGridNode(double gridSize, uint32_t divisions) const;

    vsg::ref_ptr<vsg::Group>  axisNode_;
    vsg::ref_ptr<vsg::Group>  gridNode_;
    vsg::ref_ptr<vsg::Switch> gridSwitch_;
    bool gridVisible_ = true;
};

} // namespace visualization
