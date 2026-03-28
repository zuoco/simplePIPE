#pragma once

#include <vsg/maths/vec3.h>
#include <vsg/nodes/LOD.h>

namespace visualization {

/// LOD 层级参数：minimumScreenHeightRatio 控制显示细节级别的屏占比阈值
/// VSG 的 LOD 按照 minimumScreenHeightRatio 从高到低排列子节点，
/// 当包围球占屏幕高度比例 >= 阈值时显示该层级。
struct LodLevels {
    double highDetailRatio = 0.05; ///< 高精度层显示阈值（屏占比 >= 5%）
    double lowDetailRatio  = 0.0;  ///< 低精度层显示阈值（始终可见，ratio = 0）
};

/// @brief 创建两级 VSG LOD 节点
///
/// 层次结构：
/// - 子节点 0（高精度）：当包围球屏占比 >= highDetailRatio 时显示
/// - 子节点 1（低精度）：当包围球屏占比 >= lowDetailRatio 时显示（默认始终可见）
///
/// @param highDetail  精细几何节点（近距离显示）
/// @param lowDetail   简化几何节点（远距离显示）
/// @param center      包围球中心（用于 VSG 视锥剔除和屏占比计算）
/// @param radius      包围球半径（mm）
/// @param levels      LOD 阈值参数
/// @return            vsg::LOD 节点
vsg::ref_ptr<vsg::LOD> createLodNode(
    vsg::ref_ptr<vsg::Node> highDetail,
    vsg::ref_ptr<vsg::Node> lowDetail,
    const vsg::dvec3& center,
    double radius,
    const LodLevels& levels = {});

} // namespace visualization
