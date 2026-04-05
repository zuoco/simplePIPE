// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "visualization/LodStrategy.h"

namespace visualization {

vsg::ref_ptr<vsg::LOD> createLodNode(
    vsg::ref_ptr<vsg::Node> highDetail,
    vsg::ref_ptr<vsg::Node> lowDetail,
    const vsg::dvec3& center,
    double radius,
    const LodLevels& levels)
{
    auto lod = vsg::LOD::create();

    // 设置包围球供 VSG 视锥剔除和屏占比计算
    lod->bound = vsg::dsphere{center, radius};

    // 子节点按 minimumScreenHeightRatio 从高到低排列：
    //   排在前面的子节点在屏占比更高（更近）时显示
    if (highDetail) {
        lod->addChild({levels.highDetailRatio, highDetail});
    }
    if (lowDetail) {
        lod->addChild({levels.lowDetailRatio, lowDetail});
    }

    return lod;
}

} // namespace visualization
