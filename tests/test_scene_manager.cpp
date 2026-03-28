#include <gtest/gtest.h>

#include "visualization/SceneManager.h"
#include "visualization/PipePointNode.h"
#include "visualization/ComponentNode.h"
#include "visualization/LodStrategy.h"

#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/core/Array.h>

// ══════════════════════════════════════════════════════════════════════════════
// 辅助：创建一个最简 VertexIndexDraw 用于测试
// ══════════════════════════════════════════════════════════════════════════════
namespace {

vsg::ref_ptr<vsg::VertexIndexDraw> makeDummyVid() {
    auto v = vsg::vec3Array::create(3);
    (*v)[0] = vsg::vec3{0, 0, 0};
    (*v)[1] = vsg::vec3{1, 0, 0};
    (*v)[2] = vsg::vec3{0, 1, 0};

    auto n = vsg::vec3Array::create(3);
    (*n)[0] = (*n)[1] = (*n)[2] = vsg::vec3{0, 0, 1};

    auto idx = vsg::uintArray::create(3);
    (*idx)[0] = 0; (*idx)[1] = 1; (*idx)[2] = 2;

    auto vid = vsg::VertexIndexDraw::create();
    vid->assignArrays({v, n});
    vid->assignIndices(idx);
    vid->indexCount    = 3;
    vid->instanceCount = 1;
    return vid;
}

} // anonymous namespace

// ══════════════════════════════════════════════════════════════════════════════
// T12: PipePointNode
// ══════════════════════════════════════════════════════════════════════════════

/// createPipePointNode 返回非空节点
TEST(PipePointNode, ReturnsNonNull) {
    auto node = visualization::createPipePointNode(0.0, 0.0, 0.0);
    EXPECT_NE(node, nullptr);
}

/// 节点含有至少一个子节点（立方体几何）
TEST(PipePointNode, HasChild) {
    auto node = visualization::createPipePointNode(100.0, 200.0, 300.0);
    ASSERT_NE(node, nullptr);
    EXPECT_GE(node->children.size(), 1u);
}

/// 立方体几何子节点是 VertexIndexDraw，索引数为 36（12 个三角形）
TEST(PipePointNode, BoxMeshHas36Indices) {
    auto node = visualization::createPipePointNode(0.0, 0.0, 0.0, 20.0f);
    ASSERT_NE(node, nullptr);
    ASSERT_GE(node->children.size(), 1u);

    auto vid = node->children[0].cast<vsg::VertexIndexDraw>();
    ASSERT_NE(vid, nullptr);
    EXPECT_EQ(vid->indexCount, 36u);
    EXPECT_EQ(vid->instanceCount, 1u);
}

/// 变换矩阵将节点平移到指定位置
TEST(PipePointNode, TranslationEncoded) {
    const double X = 123.0, Y = 456.0, Z = 789.0;
    auto node = visualization::createPipePointNode(X, Y, Z, 20.0f);
    ASSERT_NE(node, nullptr);

    // VSG dmat4 是列主序：mat[3] 是平移列
    const auto& m = node->matrix;
    EXPECT_NEAR(m[3][0], X, 1e-9);
    EXPECT_NEAR(m[3][1], Y, 1e-9);
    EXPECT_NEAR(m[3][2], Z, 1e-9);
}

/// 默认 size=20 与自定义 size=50 的效果不同（缩放比例）
TEST(PipePointNode, SizeAffectsScale) {
    auto n20 = visualization::createPipePointNode(0, 0, 0, 20.0f);
    auto n50 = visualization::createPipePointNode(0, 0, 0, 50.0f);
    ASSERT_NE(n20, nullptr);
    ASSERT_NE(n50, nullptr);
    // 缩放体现在矩阵对角元素
    EXPECT_NEAR(n20->matrix[0][0], 20.0, 1e-9);
    EXPECT_NEAR(n50->matrix[0][0], 50.0, 1e-9);
}

// ══════════════════════════════════════════════════════════════════════════════
// T12: ComponentNode
// ══════════════════════════════════════════════════════════════════════════════

/// createComponentNode 返回非空 MatrixTransform
TEST(ComponentNode, ReturnsNonNull) {
    auto vid = makeDummyVid();
    auto node = visualization::createComponentNode(vid);
    EXPECT_NE(node, nullptr);
}

/// 层次结构：MatrixTransform → StateGroup → VertexIndexDraw
TEST(ComponentNode, HierarchyCorrect) {
    auto vid = makeDummyVid();
    auto node = visualization::createComponentNode(vid);
    ASSERT_NE(node, nullptr);
    ASSERT_GE(node->children.size(), 1u);

    auto sg = node->children[0].cast<vsg::StateGroup>();
    ASSERT_NE(sg, nullptr);
    ASSERT_GE(sg->children.size(), 1u);

    auto vd = sg->children[0].cast<vsg::VertexIndexDraw>();
    ASSERT_NE(vd, nullptr);
    EXPECT_EQ(vd->indexCount, 3u);
}

/// nullptr vid 时，StateGroup 无子节点（但 MatrixTransform 存在）
TEST(ComponentNode, NullVidHandled) {
    auto node = visualization::createComponentNode(nullptr);
    ASSERT_NE(node, nullptr);
    ASSERT_GE(node->children.size(), 1u);

    auto sg = node->children[0].cast<vsg::StateGroup>();
    ASSERT_NE(sg, nullptr);
    EXPECT_EQ(sg->children.size(), 0u);
}

/// 变换矩阵正确编码
TEST(ComponentNode, MatrixEncoded) {
    vsg::dmat4 mat;
    mat[3][0] = 10.0;
    mat[3][1] = 20.0;
    mat[3][2] = 30.0;

    auto vid  = makeDummyVid();
    auto node = visualization::createComponentNode(vid, mat);
    ASSERT_NE(node, nullptr);

    EXPECT_NEAR(node->matrix[3][0], 10.0, 1e-9);
    EXPECT_NEAR(node->matrix[3][1], 20.0, 1e-9);
    EXPECT_NEAR(node->matrix[3][2], 30.0, 1e-9);
}

// ══════════════════════════════════════════════════════════════════════════════
// T12: LodStrategy
// ══════════════════════════════════════════════════════════════════════════════

/// createLodNode 返回非空 LOD 节点
TEST(LodStrategy, ReturnsNonNull) {
    auto high = vsg::Group::create();
    auto low  = vsg::Group::create();
    auto lod  = visualization::createLodNode(high, low, {0, 0, 0}, 100.0);
    EXPECT_NE(lod, nullptr);
}

/// LOD 节点应有两个子级别
TEST(LodStrategy, HasTwoLevels) {
    auto high = vsg::Group::create();
    auto low  = vsg::Group::create();
    auto lod  = visualization::createLodNode(high, low, {0, 0, 0}, 100.0);
    ASSERT_NE(lod, nullptr);
    EXPECT_EQ(lod->children.size(), 2u);
}

/// 高精度层的 minimumScreenHeightRatio 高于低精度层
TEST(LodStrategy, HighDetailRatioGreater) {
    auto high = vsg::Group::create();
    auto low  = vsg::Group::create();
    visualization::LodLevels levels;
    levels.highDetailRatio = 0.05;
    levels.lowDetailRatio  = 0.0;
    auto lod = visualization::createLodNode(high, low, {0, 0, 0}, 100.0, levels);
    ASSERT_NE(lod, nullptr);
    ASSERT_EQ(lod->children.size(), 2u);

    EXPECT_DOUBLE_EQ(lod->children[0].minimumScreenHeightRatio, 0.05);
    EXPECT_DOUBLE_EQ(lod->children[1].minimumScreenHeightRatio, 0.0);
}

/// 包围球中心和半径正确设置
TEST(LodStrategy, BoundSphereCorrect) {
    auto high = vsg::Group::create();
    auto low  = vsg::Group::create();
    auto lod  = visualization::createLodNode(high, low, {1.0, 2.0, 3.0}, 50.0);
    ASSERT_NE(lod, nullptr);

    EXPECT_NEAR(lod->bound.center.x, 1.0, 1e-9);
    EXPECT_NEAR(lod->bound.center.y, 2.0, 1e-9);
    EXPECT_NEAR(lod->bound.center.z, 3.0, 1e-9);
    EXPECT_NEAR(lod->bound.radius,  50.0, 1e-9);
}

/// 自定义 LOD 阈值生效
TEST(LodStrategy, CustomLevels) {
    auto high = vsg::Group::create();
    auto low  = vsg::Group::create();
    visualization::LodLevels levels;
    levels.highDetailRatio = 0.1;
    levels.lowDetailRatio  = 0.02;
    auto lod = visualization::createLodNode(high, low, {0, 0, 0}, 10.0, levels);
    ASSERT_NE(lod, nullptr);
    ASSERT_EQ(lod->children.size(), 2u);
    EXPECT_DOUBLE_EQ(lod->children[0].minimumScreenHeightRatio, 0.1);
    EXPECT_DOUBLE_EQ(lod->children[1].minimumScreenHeightRatio, 0.02);
}

// ══════════════════════════════════════════════════════════════════════════════
// T12: SceneManager
// ══════════════════════════════════════════════════════════════════════════════

/// SceneManager 初始状态：根节点有效，节点数为 0
TEST(SceneManager, InitialState) {
    visualization::SceneManager mgr;
    EXPECT_NE(mgr.root(), nullptr);
    EXPECT_EQ(mgr.nodeCount(), 0u);
}

/// addNode 后，根节点 children 数量增加
TEST(SceneManager, AddNodeUpdatesRoot) {
    visualization::SceneManager mgr;
    auto node = visualization::createPipePointNode(0, 0, 0);
    mgr.addNode("uuid-001", node);

    EXPECT_EQ(mgr.nodeCount(), 1u);
    EXPECT_EQ(mgr.root()->children.size(), 1u);
}

/// addNode 后，findNode 能通过 UUID 找到节点
TEST(SceneManager, FindNodeByUuid) {
    visualization::SceneManager mgr;
    auto node = visualization::createPipePointNode(10, 20, 30);
    mgr.addNode("uuid-001", node);

    auto found = mgr.findNode("uuid-001");
    EXPECT_EQ(found, node);
}

/// findNode 不存在的 UUID 返回 nullptr
TEST(SceneManager, FindNonExistentReturnsNull) {
    visualization::SceneManager mgr;
    auto found = mgr.findNode("non-existent");
    EXPECT_EQ(found, nullptr);
}

/// removeNode 后，节点数减少且根节点 children 更新
TEST(SceneManager, RemoveNodeUpdatesRoot) {
    visualization::SceneManager mgr;
    auto node = visualization::createPipePointNode(0, 0, 0);
    mgr.addNode("uuid-001", node);

    bool removed = mgr.removeNode("uuid-001");
    EXPECT_TRUE(removed);
    EXPECT_EQ(mgr.nodeCount(), 0u);
    EXPECT_EQ(mgr.root()->children.size(), 0u);
}

/// removeNode 不存在的 UUID 返回 false
TEST(SceneManager, RemoveNonExistentReturnsFalse) {
    visualization::SceneManager mgr;
    EXPECT_FALSE(mgr.removeNode("no-such-uuid"));
}

/// removeNode 后，findNode 返回 nullptr
TEST(SceneManager, FindAfterRemoveReturnsNull) {
    visualization::SceneManager mgr;
    auto node = visualization::createPipePointNode(0, 0, 0);
    mgr.addNode("uuid-001", node);
    mgr.removeNode("uuid-001");

    EXPECT_EQ(mgr.findNode("uuid-001"), nullptr);
}

/// updateNode 替换几何后，findNode 返回新节点
TEST(SceneManager, UpdateNodeReplacesGeometry) {
    visualization::SceneManager mgr;
    auto old = visualization::createPipePointNode(0, 0, 0, 20.0f);
    mgr.addNode("uuid-001", old);

    auto newNode = visualization::createPipePointNode(1, 2, 3, 30.0f);
    bool updated = mgr.updateNode("uuid-001", newNode);

    EXPECT_TRUE(updated);
    EXPECT_EQ(mgr.findNode("uuid-001"), newNode);
    EXPECT_NE(mgr.findNode("uuid-001"), old);
}

/// updateNode 后根节点 children 中是新节点
TEST(SceneManager, UpdateNodeRefreshesRootChildren) {
    visualization::SceneManager mgr;
    auto old = visualization::createPipePointNode(0, 0, 0, 20.0f);
    mgr.addNode("uuid-001", old);

    auto newNode = visualization::createPipePointNode(5, 6, 7, 25.0f);
    mgr.updateNode("uuid-001", newNode);

    ASSERT_EQ(mgr.root()->children.size(), 1u);
    EXPECT_EQ(mgr.root()->children[0], newNode);
}

/// updateNode 对不存在 UUID 返回 false
TEST(SceneManager, UpdateNonExistentReturnsFalse) {
    visualization::SceneManager mgr;
    auto node = visualization::createPipePointNode(0, 0, 0);
    EXPECT_FALSE(mgr.updateNode("no-uuid", node));
}

/// batchUpdate 批量替换多个节点
TEST(SceneManager, BatchUpdateMultipleNodes) {
    visualization::SceneManager mgr;
    mgr.addNode("a", visualization::createPipePointNode(0, 0, 0));
    mgr.addNode("b", visualization::createPipePointNode(1, 0, 0));
    mgr.addNode("c", visualization::createPipePointNode(2, 0, 0));

    auto newA = visualization::createPipePointNode(10, 0, 0);
    auto newC = visualization::createPipePointNode(30, 0, 0);

    mgr.batchUpdate({{"a", newA}, {"c", newC}});

    EXPECT_EQ(mgr.findNode("a"), newA);
    EXPECT_EQ(mgr.findNode("c"), newC);
    EXPECT_EQ(mgr.nodeCount(), 3u); // "b" 未改变
}

/// batchUpdate 后根节点 children 数量不变
TEST(SceneManager, BatchUpdatePreservesCount) {
    visualization::SceneManager mgr;
    mgr.addNode("x", visualization::createPipePointNode(0, 0, 0));
    mgr.addNode("y", visualization::createPipePointNode(1, 0, 0));

    auto n1 = visualization::createPipePointNode(5, 5, 5);
    auto n2 = visualization::createPipePointNode(6, 6, 6);
    mgr.batchUpdate({{"x", n1}, {"y", n2}});

    EXPECT_EQ(mgr.root()->children.size(), 2u);
}

/// 多对象混合：add + remove + update 顺序操作保持一致
TEST(SceneManager, MixedOperationsConsistent) {
    visualization::SceneManager mgr;
    mgr.addNode("a", visualization::createPipePointNode(0, 0, 0));
    mgr.addNode("b", visualization::createPipePointNode(1, 0, 0));
    mgr.addNode("c", visualization::createPipePointNode(2, 0, 0));

    mgr.removeNode("b");
    EXPECT_EQ(mgr.nodeCount(), 2u);
    EXPECT_EQ(mgr.root()->children.size(), 2u);

    auto newA = visualization::createPipePointNode(99, 0, 0);
    mgr.updateNode("a", newA);
    EXPECT_EQ(mgr.findNode("a"), newA);

    mgr.addNode("d", visualization::createPipePointNode(3, 0, 0));
    EXPECT_EQ(mgr.nodeCount(), 3u);
    EXPECT_EQ(mgr.root()->children.size(), 3u);
}

/// 重复 addNode 相同 UUID 不会重复插入
TEST(SceneManager, DuplicateAddIgnored) {
    visualization::SceneManager mgr;
    auto n1 = visualization::createPipePointNode(0, 0, 0);
    auto n2 = visualization::createPipePointNode(1, 0, 0);

    mgr.addNode("dup", n1);
    mgr.addNode("dup", n2); // 应被忽略

    EXPECT_EQ(mgr.nodeCount(), 1u);
    EXPECT_EQ(mgr.root()->children.size(), 1u);
    EXPECT_EQ(mgr.findNode("dup"), n1); // 仍是第一个节点
}

/// hasNode 正确报告节点是否存在
TEST(SceneManager, HasNodeCheck) {
    visualization::SceneManager mgr;
    EXPECT_FALSE(mgr.hasNode("uuid-X"));
    mgr.addNode("uuid-X", visualization::createPipePointNode(0, 0, 0));
    EXPECT_TRUE(mgr.hasNode("uuid-X"));
    mgr.removeNode("uuid-X");
    EXPECT_FALSE(mgr.hasNode("uuid-X"));
}
