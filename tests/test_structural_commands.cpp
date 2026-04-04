// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

/// @file test_structural_commands.cpp
/// T8 — CreatePipePointCommand / DeletePipePointCommand 单元测试

#include <gtest/gtest.h>

#include "command/CreatePipePointCommand.h"
#include "command/DeletePipePointCommand.h"
#include "command/CommandContext.h"
#include "command/CommandStack.h"
#include "command/CommandRegistry.h"
#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "engine/TopologyManager.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Route.h"
#include "model/Segment.h"

#include <memory>

// ============================================================
// 测试夹具：创建带 Route/Segment 的基本文档环境
// ============================================================

class StructuralCommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        doc = std::make_unique<app::Document>();
        depGraph = std::make_unique<app::DependencyGraph>();
        topoMgr = std::make_unique<engine::TopologyManager>();

        // 创建路由和段
        route = std::make_shared<model::Route>("Route_01");
        seg = std::make_shared<model::Segment>("Seg_01");
        route->addSegment(seg);
        doc->addObject(route);
        doc->addObject(seg);

        routeId = route->id();
        segId = seg->id();

        ctx.document = doc.get();
        ctx.dependencyGraph = depGraph.get();
        ctx.topologyManager = topoMgr.get();
    }

    std::unique_ptr<app::Document> doc;
    std::unique_ptr<app::DependencyGraph> depGraph;
    std::unique_ptr<engine::TopologyManager> topoMgr;

    std::shared_ptr<model::Route> route;
    std::shared_ptr<model::Segment> seg;
    foundation::UUID routeId;
    foundation::UUID segId;

    command::CommandContext ctx;
};

// ============================================================
// CreatePipePointCommand — 基础 execute / undo / redo
// ============================================================

TEST_F(StructuralCommandsTest, Create_Execute_AddsPointToDocument) {
    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run,
        1000.0, 0.0, 0.0);

    cmd->execute(ctx);

    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_EQ(cmd->lastResult().createdIds.size(), 1u);

    // 管点已在文档中
    auto* pp = doc->findObject(cmd->createdPointId());
    ASSERT_NE(pp, nullptr);
    EXPECT_EQ(pp->name(), "PP_001");

    // 管点已在段中
    EXPECT_EQ(seg->pointCount(), 1u);
    EXPECT_EQ(seg->pointAt(0)->id(), cmd->createdPointId());
}

TEST_F(StructuralCommandsTest, Create_Undo_RemovesPoint) {
    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run,
        1000.0, 0.0, 0.0);

    cmd->execute(ctx);
    auto pointId = cmd->createdPointId();

    cmd->undo(ctx);

    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_EQ(cmd->lastResult().deletedIds.size(), 1u);
    EXPECT_EQ(cmd->lastResult().deletedIds[0], pointId);

    // 文档中已无该管点
    EXPECT_EQ(doc->findObject(pointId), nullptr);
    // 段中已无管点
    EXPECT_EQ(seg->pointCount(), 0u);
}

TEST_F(StructuralCommandsTest, Create_Redo_RestoresWithSameUUID) {
    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run,
        1000.0, 0.0, 0.0);

    cmd->execute(ctx);
    auto pointId = cmd->createdPointId();

    cmd->undo(ctx);

    // redo（再次 execute）
    cmd->execute(ctx);

    // UUID 不变
    EXPECT_EQ(cmd->createdPointId(), pointId);
    auto* pp = doc->findObject(pointId);
    ASSERT_NE(pp, nullptr);
    EXPECT_EQ(pp->name(), "PP_001");
    EXPECT_EQ(seg->pointCount(), 1u);
}

TEST_F(StructuralCommandsTest, Create_WithInsertIndex) {
    // 先创建两个管点
    auto cmd1 = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run, 0.0, 0.0, 0.0);
    cmd1->execute(ctx);

    auto cmd2 = command::CreatePipePointCommand::create(
        routeId, segId, "PP_003", model::PipePointType::Run, 2000.0, 0.0, 0.0);
    cmd2->execute(ctx);

    EXPECT_EQ(seg->pointCount(), 2u);

    // 在索引1处插入
    auto cmd3 = command::CreatePipePointCommand::create(
        routeId, segId, "PP_002", model::PipePointType::Run,
        1000.0, 0.0, 0.0, "", 1);
    cmd3->execute(ctx);

    EXPECT_EQ(seg->pointCount(), 3u);
    EXPECT_EQ(seg->pointAt(1)->name(), "PP_002");
}

TEST_F(StructuralCommandsTest, Create_DependencyGraph_Registration) {
    // 先创建两个管点
    auto cmd1 = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run, 0.0, 0.0, 0.0);
    cmd1->execute(ctx);
    auto id1 = cmd1->createdPointId();

    auto cmd2 = command::CreatePipePointCommand::create(
        routeId, segId, "PP_003", model::PipePointType::Run, 2000.0, 0.0, 0.0);
    cmd2->execute(ctx);
    auto id2 = cmd2->createdPointId();

    // 中间插入一个管点：应与 PP_001 和 PP_003 建立依赖
    auto cmd3 = command::CreatePipePointCommand::create(
        routeId, segId, "PP_002", model::PipePointType::Run,
        1000.0, 0.0, 0.0, "", 1);
    cmd3->execute(ctx);
    auto id3 = cmd3->createdPointId();

    // 标脏 PP_001，应传播到 PP_002（PP_002 依赖 PP_001）
    depGraph->markDirty(id1);
    EXPECT_TRUE(depGraph->isDirty(id3));

    depGraph->clearDirty();

    // 标脏 PP_003，应传播到 PP_002
    depGraph->markDirty(id2);
    EXPECT_TRUE(depGraph->isDirty(id3));
}

TEST_F(StructuralCommandsTest, Create_Undo_RemovesDependency) {
    auto cmd1 = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run, 0.0, 0.0, 0.0);
    cmd1->execute(ctx);
    auto id1 = cmd1->createdPointId();

    auto cmd2 = command::CreatePipePointCommand::create(
        routeId, segId, "PP_002", model::PipePointType::Run, 1000.0, 0.0, 0.0);
    cmd2->execute(ctx);
    auto id2 = cmd2->createdPointId();

    // undo PP_002
    cmd2->undo(ctx);

    // 标脏 PP_001，PP_002 的依赖已移除，不应脏
    depGraph->clearDirty();
    depGraph->markDirty(id1);
    EXPECT_FALSE(depGraph->isDirty(id2));
}

// ============================================================
// CreatePipePointCommand — Tee 分支段
// ============================================================

TEST_F(StructuralCommandsTest, Create_Tee_CreatesBranchSegment) {
    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "TEE_001", model::PipePointType::Tee,
        1000.0, 0.0, 0.0);

    cmd->execute(ctx);

    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_FALSE(cmd->createdBranchId().isNull());

    // 路由中应有 2 个段（原段 + 分支段）
    EXPECT_EQ(route->segmentCount(), 2u);

    // 分支段在文档中
    auto* branchSeg = doc->findObject(cmd->createdBranchId());
    EXPECT_NE(branchSeg, nullptr);
}

TEST_F(StructuralCommandsTest, Create_Tee_Undo_RemovesBranchSegment) {
    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "TEE_001", model::PipePointType::Tee,
        1000.0, 0.0, 0.0);

    cmd->execute(ctx);
    auto branchId = cmd->createdBranchId();

    cmd->undo(ctx);

    // 分支段已从路由和文档中移除
    EXPECT_EQ(route->segmentCount(), 1u);
    EXPECT_EQ(doc->findObject(branchId), nullptr);
}

TEST_F(StructuralCommandsTest, Create_Tee_Redo_RestoresBranch) {
    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "TEE_001", model::PipePointType::Tee,
        1000.0, 0.0, 0.0);

    cmd->execute(ctx);
    auto pointId = cmd->createdPointId();

    cmd->undo(ctx);
    cmd->execute(ctx);

    // UUID 稳定
    EXPECT_EQ(cmd->createdPointId(), pointId);
    EXPECT_EQ(route->segmentCount(), 2u);
}

// ============================================================
// CreatePipePointCommand — PipeSpec 关联
// ============================================================

TEST_F(StructuralCommandsTest, Create_WithPipeSpec) {
    auto spec = std::make_shared<model::PipeSpec>("6inch_CS");
    spec->setOd(168.3);
    spec->setWallThickness(7.11);
    doc->addObject(spec);

    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run,
        1000.0, 0.0, 0.0,
        spec->id().toString());

    cmd->execute(ctx);

    auto* ppObj = doc->findObject(cmd->createdPointId());
    auto* pp = dynamic_cast<model::PipePoint*>(ppObj);
    ASSERT_NE(pp, nullptr);
    EXPECT_NE(pp->pipeSpec(), nullptr);
    EXPECT_EQ(pp->pipeSpec()->name(), "6inch_CS");
}

// ============================================================
// CreatePipePointCommand — JSON 序列化 round-trip
// ============================================================

TEST_F(StructuralCommandsTest, Create_JsonRoundTrip) {
    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Bend,
        1500.0, 200.0, 300.0);

    cmd->execute(ctx);

    // 序列化
    auto j = cmd->toJson();
    EXPECT_EQ(j["type"], "CreatePipePoint");
    EXPECT_EQ(j["name"], "PP_001");
    EXPECT_EQ(j["pointType"], "Bend");

    // 反序列化
    auto cmd2 = command::CreatePipePointCommand::fromJson(j);
    EXPECT_EQ(cmd2->pointName(), "PP_001");
    EXPECT_EQ(cmd2->pointType(), model::PipePointType::Bend);
    EXPECT_EQ(cmd2->routeId(), routeId);
}

TEST_F(StructuralCommandsTest, Create_RegistryRoundTrip) {
    command::CommandRegistry registry;
    registry.registerBuiltins();

    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run,
        1000.0, 0.0, 0.0);
    cmd->execute(ctx);

    auto j = cmd->toJson();
    auto cmd2 = registry.deserialize(j);
    EXPECT_EQ(cmd2->type(), command::CommandType::CreatePipePoint);
}

// ============================================================
// DeletePipePointCommand — 基础 execute / undo / redo
// ============================================================

TEST_F(StructuralCommandsTest, Delete_Execute_RemovesPoint) {
    // 先创建管点
    auto pp = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Run, gp_Pnt(1000.0, 0.0, 0.0));
    auto ppId = pp->id();
    doc->addObject(pp);
    topoMgr->appendPoint(*route, *seg, pp);

    EXPECT_EQ(seg->pointCount(), 1u);

    // 删除
    auto cmd = command::DeletePipePointCommand::create(ppId);
    cmd->execute(ctx);

    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_EQ(cmd->lastResult().deletedIds.size(), 1u);
    EXPECT_EQ(doc->findObject(ppId), nullptr);
    EXPECT_EQ(seg->pointCount(), 0u);
}

TEST_F(StructuralCommandsTest, Delete_Undo_RestoresPoint) {
    auto pp = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Bend, gp_Pnt(1000.0, 200.0, 300.0));
    pp->setParam("bendMultiplier", foundation::Variant{1.5});
    auto ppId = pp->id();
    doc->addObject(pp);
    topoMgr->appendPoint(*route, *seg, pp);

    auto cmd = command::DeletePipePointCommand::create(ppId);
    cmd->execute(ctx);

    // undo
    cmd->undo(ctx);

    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_EQ(cmd->lastResult().createdIds.size(), 1u);
    EXPECT_EQ(cmd->lastResult().createdIds[0], ppId);

    // 管点恢复到文档
    auto* restored = doc->findObject(ppId);
    ASSERT_NE(restored, nullptr);
    auto* rpipe = dynamic_cast<model::PipePoint*>(restored);
    ASSERT_NE(rpipe, nullptr);

    // 属性恢复
    EXPECT_EQ(rpipe->name(), "PP_001");
    EXPECT_EQ(rpipe->type(), model::PipePointType::Bend);
    EXPECT_NEAR(rpipe->position().X(), 1000.0, 1e-6);
    EXPECT_NEAR(rpipe->position().Y(), 200.0, 1e-6);
    EXPECT_NEAR(rpipe->position().Z(), 300.0, 1e-6);

    // typeParams 恢复
    EXPECT_TRUE(rpipe->hasParam("bendMultiplier"));
    EXPECT_NEAR(foundation::variantToDouble(rpipe->param("bendMultiplier")), 1.5, 1e-6);

    // 管点在段中
    EXPECT_EQ(seg->pointCount(), 1u);
}

TEST_F(StructuralCommandsTest, Delete_Undo_UUID_Stability) {
    auto pp = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Run, gp_Pnt(1000.0, 0.0, 0.0));
    auto ppId = pp->id();
    doc->addObject(pp);
    topoMgr->appendPoint(*route, *seg, pp);

    auto cmd = command::DeletePipePointCommand::create(ppId);
    cmd->execute(ctx);
    cmd->undo(ctx);

    // UUID 稳定：undo 后对象 UUID 与原始 UUID 完全一致
    auto* restored = doc->findObject(ppId);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->id(), ppId);
}

TEST_F(StructuralCommandsTest, Delete_Redo_RemovesAgain) {
    auto pp = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Run, gp_Pnt(1000.0, 0.0, 0.0));
    auto ppId = pp->id();
    doc->addObject(pp);
    topoMgr->appendPoint(*route, *seg, pp);

    auto cmd = command::DeletePipePointCommand::create(ppId);
    cmd->execute(ctx);
    cmd->undo(ctx);
    cmd->execute(ctx); // redo

    EXPECT_EQ(doc->findObject(ppId), nullptr);
    EXPECT_EQ(seg->pointCount(), 0u);
}

// ============================================================
// DeletePipePointCommand — 中间管点删除，验证段索引恢复
// ============================================================

TEST_F(StructuralCommandsTest, Delete_MiddlePoint_RestoresIndex) {
    // 创建三个管点
    auto pp1 = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Run, gp_Pnt(0.0, 0.0, 0.0));
    auto pp2 = std::make_shared<model::PipePoint>("PP_002",
        model::PipePointType::Bend, gp_Pnt(1000.0, 0.0, 0.0));
    auto pp3 = std::make_shared<model::PipePoint>("PP_003",
        model::PipePointType::Run, gp_Pnt(2000.0, 0.0, 0.0));

    doc->addObject(pp1);
    doc->addObject(pp2);
    doc->addObject(pp3);
    topoMgr->appendPoint(*route, *seg, pp1);
    topoMgr->appendPoint(*route, *seg, pp2);
    topoMgr->appendPoint(*route, *seg, pp3);

    // 删除中间管点
    auto cmd = command::DeletePipePointCommand::create(pp2->id());
    cmd->execute(ctx);

    EXPECT_EQ(seg->pointCount(), 2u);
    EXPECT_EQ(seg->pointAt(0)->name(), "PP_001");
    EXPECT_EQ(seg->pointAt(1)->name(), "PP_003");

    // undo：PP_002 应恢复到索引 1
    cmd->undo(ctx);

    EXPECT_EQ(seg->pointCount(), 3u);
    EXPECT_EQ(seg->pointAt(0)->name(), "PP_001");
    EXPECT_EQ(seg->pointAt(1)->name(), "PP_002");
    EXPECT_EQ(seg->pointAt(2)->name(), "PP_003");
}

// ============================================================
// DeletePipePointCommand — Tee 管点删除与恢复
// ============================================================

TEST_F(StructuralCommandsTest, Delete_Tee_RemovesAndRestoresBranch) {
    // 创建 Tee 管点
    auto tee = std::make_shared<model::PipePoint>("TEE_001",
        model::PipePointType::Tee, gp_Pnt(1000.0, 0.0, 0.0));
    auto teeId = tee->id();
    doc->addObject(tee);
    auto branch = topoMgr->appendPoint(*route, *seg, tee);

    ASSERT_NE(branch, nullptr);
    doc->addObject(branch);
    auto branchId = branch->id();

    EXPECT_EQ(route->segmentCount(), 2u);

    // 删除 Tee
    auto cmd = command::DeletePipePointCommand::create(teeId);
    cmd->execute(ctx);

    EXPECT_EQ(doc->findObject(teeId), nullptr);
    EXPECT_EQ(route->segmentCount(), 1u);

    // undo
    cmd->undo(ctx);

    // Tee 恢复
    EXPECT_NE(doc->findObject(teeId), nullptr);
    EXPECT_EQ(seg->pointCount(), 1u);
    EXPECT_EQ(seg->pointAt(0)->id(), teeId);

    // 分支段恢复（新创建的分支段 UUID 可能不同，但存在即可）
    EXPECT_EQ(route->segmentCount(), 2u);
}

// ============================================================
// DeletePipePointCommand — 附属构件恢复
// ============================================================

TEST_F(StructuralCommandsTest, Delete_RestoresAccessories) {
    auto pp = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Run, gp_Pnt(1000.0, 0.0, 0.0));
    auto ppId = pp->id();

    // 添加附属构件
    auto acc = std::make_shared<model::DocumentObject>("Flange_01");
    auto accId = acc->id();
    pp->addAccessory(acc);

    doc->addObject(pp);
    topoMgr->appendPoint(*route, *seg, pp);

    auto cmd = command::DeletePipePointCommand::create(ppId);
    cmd->execute(ctx);
    cmd->undo(ctx);

    // 管点恢复
    auto* restored = dynamic_cast<model::PipePoint*>(doc->findObject(ppId));
    ASSERT_NE(restored, nullptr);

    // 附属构件恢复（UUID 稳定）
    EXPECT_EQ(restored->accessoryCount(), 1u);
    EXPECT_EQ(restored->accessories()[0]->id(), accId);
    EXPECT_EQ(restored->accessories()[0]->name(), "Flange_01");
}

// ============================================================
// DeletePipePointCommand — DependencyGraph 恢复
// ============================================================

TEST_F(StructuralCommandsTest, Delete_DependencyGraph_Restoration) {
    auto pp1 = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Run, gp_Pnt(0.0, 0.0, 0.0));
    auto pp2 = std::make_shared<model::PipePoint>("PP_002",
        model::PipePointType::Run, gp_Pnt(1000.0, 0.0, 0.0));

    doc->addObject(pp1);
    doc->addObject(pp2);
    topoMgr->appendPoint(*route, *seg, pp1);
    topoMgr->appendPoint(*route, *seg, pp2);

    // 手动注册依赖
    depGraph->addDependency(pp2->id(), pp1->id());

    auto id1 = pp1->id();
    auto id2 = pp2->id();

    // 删除 PP_002
    auto cmd = command::DeletePipePointCommand::create(id2);
    cmd->execute(ctx);

    // DependencyGraph 中 PP_002 的依赖已清除
    depGraph->clearDirty();
    depGraph->markDirty(id1);
    EXPECT_FALSE(depGraph->isDirty(id2));

    // undo — 恢复 PP_002 及其依赖
    cmd->undo(ctx);

    depGraph->clearDirty();
    depGraph->markDirty(id1);
    EXPECT_TRUE(depGraph->isDirty(id2));
}

// ============================================================
// DeletePipePointCommand — JSON 序列化 round-trip
// ============================================================

TEST_F(StructuralCommandsTest, Delete_JsonRoundTrip) {
    auto pp = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Bend, gp_Pnt(1000.0, 200.0, 300.0));
    pp->setParam("bendMultiplier", foundation::Variant{1.5});
    auto ppId = pp->id();
    doc->addObject(pp);
    topoMgr->appendPoint(*route, *seg, pp);

    auto cmd = command::DeletePipePointCommand::create(ppId);
    cmd->execute(ctx);

    // 序列化
    auto j = cmd->toJson();
    EXPECT_EQ(j["type"], "DeletePipePoint");
    EXPECT_TRUE(j.contains("savedState"));
    EXPECT_EQ(j["savedState"]["name"], "PP_001");
    EXPECT_EQ(j["savedState"]["type"], "Bend");

    // 反序列化
    auto cmd2 = command::DeletePipePointCommand::fromJson(j);
    EXPECT_EQ(cmd2->pointId(), ppId);
    EXPECT_EQ(cmd2->savedState().name, "PP_001");
    EXPECT_EQ(cmd2->savedState().type, model::PipePointType::Bend);
}

TEST_F(StructuralCommandsTest, Delete_RegistryRoundTrip) {
    command::CommandRegistry registry;
    registry.registerBuiltins();

    auto pp = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Run, gp_Pnt(1000.0, 0.0, 0.0));
    auto ppId = pp->id();
    doc->addObject(pp);
    topoMgr->appendPoint(*route, *seg, pp);

    auto cmd = command::DeletePipePointCommand::create(ppId);
    cmd->execute(ctx);

    auto j = cmd->toJson();
    auto cmd2 = registry.deserialize(j);
    EXPECT_EQ(cmd2->type(), command::CommandType::DeletePipePoint);
}

// ============================================================
// CommandStack 集成：CreatePipePoint + DeletePipePoint
// ============================================================

TEST_F(StructuralCommandsTest, CommandStack_CreateUndoRedo) {
    command::CommandStack stack;

    auto cmd = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run,
        1000.0, 0.0, 0.0);

    auto result = stack.execute(std::move(cmd), ctx);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(seg->pointCount(), 1u);

    stack.undo(ctx);
    EXPECT_EQ(seg->pointCount(), 0u);

    stack.redo(ctx);
    EXPECT_EQ(seg->pointCount(), 1u);
}

TEST_F(StructuralCommandsTest, CommandStack_DeleteUndoRedo) {
    auto pp = std::make_shared<model::PipePoint>("PP_001",
        model::PipePointType::Run, gp_Pnt(1000.0, 0.0, 0.0));
    auto ppId = pp->id();
    doc->addObject(pp);
    topoMgr->appendPoint(*route, *seg, pp);

    command::CommandStack stack;

    auto cmd = command::DeletePipePointCommand::create(ppId);
    auto result = stack.execute(std::move(cmd), ctx);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(doc->findObject(ppId), nullptr);

    stack.undo(ctx);
    EXPECT_NE(doc->findObject(ppId), nullptr);
    EXPECT_EQ(seg->pointCount(), 1u);

    stack.redo(ctx);
    EXPECT_EQ(doc->findObject(ppId), nullptr);
}

TEST_F(StructuralCommandsTest, CommandStack_CreateThenDelete_FullCycle) {
    command::CommandStack stack;

    // 创建
    auto createCmd = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run,
        1000.0, 0.0, 0.0);
    stack.execute(std::move(createCmd), ctx);
    EXPECT_EQ(seg->pointCount(), 1u);

    auto ppId = seg->pointAt(0)->id();

    // 删除
    auto delCmd = command::DeletePipePointCommand::create(ppId);
    stack.execute(std::move(delCmd), ctx);
    EXPECT_EQ(seg->pointCount(), 0u);
    EXPECT_EQ(doc->findObject(ppId), nullptr);

    // undo delete → 管点恢复
    stack.undo(ctx);
    EXPECT_EQ(seg->pointCount(), 1u);
    EXPECT_NE(doc->findObject(ppId), nullptr);

    // undo create → 管点消失
    stack.undo(ctx);
    EXPECT_EQ(seg->pointCount(), 0u);
    EXPECT_EQ(doc->findObject(ppId), nullptr);

    // redo create → 管点恢复
    stack.redo(ctx);
    EXPECT_EQ(seg->pointCount(), 1u);
    EXPECT_NE(doc->findObject(ppId), nullptr);
}

// ============================================================
// CommandStack — sceneRemoveRequested 信号
// ============================================================

TEST_F(StructuralCommandsTest, CommandStack_SceneRemoveSignal) {
    command::CommandStack stack;
    std::vector<std::string> removedIds;

    stack.sceneRemoveRequested.connect([&](const std::string& uuid) {
        removedIds.push_back(uuid);
    });

    // 创建管点
    auto createCmd = command::CreatePipePointCommand::create(
        routeId, segId, "PP_001", model::PipePointType::Run, 1000.0, 0.0, 0.0);
    stack.execute(std::move(createCmd), ctx);
    auto ppId = seg->pointAt(0)->id();

    // undo create → 应该触发 sceneRemoveRequested
    stack.undo(ctx);
    EXPECT_EQ(removedIds.size(), 1u);

    removedIds.clear();

    // redo create → 不触发 sceneRemoveRequested
    stack.redo(ctx);
    EXPECT_EQ(removedIds.size(), 0u);
}
