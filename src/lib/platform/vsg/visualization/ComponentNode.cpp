// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "visualization/ComponentNode.h"

#include <vsg/nodes/StateGroup.h>

namespace visualization {

vsg::ref_ptr<vsg::MatrixTransform> createComponentNode(
    vsg::ref_ptr<vsg::VertexIndexDraw> vid, const vsg::dmat4& matrix)
{
    // MatrixTransform 承载世界变换
    auto transform = vsg::MatrixTransform::create();
    transform->matrix = matrix;

    // StateGroup 留空（stateCommands 由渲染层在 compile 前填充）
    auto stateGroup = vsg::StateGroup::create();

    if (vid) {
        stateGroup->addChild(vid);
    }

    transform->addChild(stateGroup);
    return transform;
}

} // namespace visualization
