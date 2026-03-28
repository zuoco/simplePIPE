#include "visualization/SceneFurniture.h"

#include <vsg/core/Array.h>
#include <vsg/nodes/VertexDraw.h>

namespace visualization {

namespace {

/// 构建 VertexDraw 并存储给定的顶点列表（用于轴线或网格线段）
vsg::ref_ptr<vsg::VertexDraw> makeLinesNode(const std::vector<vsg::vec3>& pts) {
    auto verts = vsg::vec3Array::create(static_cast<uint32_t>(pts.size()));
    for (size_t i = 0; i < pts.size(); ++i)
        (*verts)[i] = pts[i];

    auto vd = vsg::VertexDraw::create();
    vd->assignArrays({verts});
    vd->vertexCount    = static_cast<uint32_t>(pts.size());
    vd->instanceCount  = 1;
    vd->firstVertex    = 0;
    vd->firstInstance  = 0;
    return vd;
}

} // anonymous namespace

SceneFurniture::SceneFurniture(double axisLength, double gridSize, uint32_t gridDivisions) {
    axisNode_  = buildAxisNode(axisLength);
    gridNode_  = buildGridNode(gridSize, gridDivisions);
    gridSwitch_ = vsg::Switch::create();
    gridSwitch_->addChild(true, gridNode_);
    gridVisible_ = true;
}

vsg::ref_ptr<vsg::Group> SceneFurniture::buildAxisNode(double axisLength) const {
    auto group = vsg::Group::create();
    const auto L = static_cast<float>(axisLength);

    // X 轴（红）: (0,0,0) → (L,0,0)
    group->addChild(makeLinesNode({ {0.f,0.f,0.f}, {L,0.f,0.f} }));
    // Y 轴（绿）: (0,0,0) → (0,L,0)
    group->addChild(makeLinesNode({ {0.f,0.f,0.f}, {0.f,L,0.f} }));
    // Z 轴（蓝）: (0,0,0) → (0,0,L)
    group->addChild(makeLinesNode({ {0.f,0.f,0.f}, {0.f,0.f,L} }));
    return group;
}

vsg::ref_ptr<vsg::Group> SceneFurniture::buildGridNode(double gridSize, uint32_t divisions) const {
    auto group = vsg::Group::create();
    if (divisions == 0) return group;

    const auto  half = static_cast<float>(gridSize * 0.5);
    const float step = static_cast<float>(gridSize) / static_cast<float>(divisions);

    std::vector<vsg::vec3> pts;
    pts.reserve(static_cast<size_t>((divisions + 1)) * 4);

    for (uint32_t i = 0; i <= divisions; ++i) {
        float t = -half + step * static_cast<float>(i);
        // 平行于 X 轴的线（y = t）
        pts.push_back({ -half, t, 0.f });
        pts.push_back({  half, t, 0.f });
        // 平行于 Y 轴的线（x = t）
        pts.push_back({ t, -half, 0.f });
        pts.push_back({ t,  half, 0.f });
    }

    group->addChild(makeLinesNode(pts));
    return group;
}

vsg::ref_ptr<vsg::Group>  SceneFurniture::axisNode()  const { return axisNode_; }
vsg::ref_ptr<vsg::Group>  SceneFurniture::gridNode()  const { return gridNode_; }
vsg::ref_ptr<vsg::Switch> SceneFurniture::gridSwitch() const { return gridSwitch_; }

void SceneFurniture::setGridVisible(bool visible) {
    gridVisible_ = visible;
    gridSwitch_->setAllChildren(visible);
}

bool SceneFurniture::isGridVisible() const { return gridVisible_; }

} // namespace visualization
