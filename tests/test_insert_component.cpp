// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

/// @file test_insert_component.cpp
/// T9 — InsertComponentCommand 单元测试

#include <gtest/gtest.h>

#include "command/InsertComponentCommand.h"
#include "command/CreatePipePointCommand.h"
#include "command/DeletePipePointCommand.h"
#include "command/CommandContext.h"
#include "command/CommandStack.h"
#include "command/CommandRegistry.h"
#include "command/MacroCommand.h"
#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "engine/TopologyManager.h"
#include "model/PipePoint.h"
#include "model/Route.h"
#include "model/Segment.h"

#include <memory>

// ============================================================
// 测试夹具
// ============================================================

class InsertComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        doc = std::make_unique<app::Document>();
        depGraph = std::make_unique<app::DependencyGraph>();
        topoMgr = std::make_unique<engine::TopologyManager>();

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
// mapComponentType 映射测试
// ============================================================

TEST(InsertComponentMapping, KnownTypes)
{
    using command::InsertComponentCommand;
    using model::PipePointType;

    EXPECT_EQ(InsertComponentCommand::mapComponentType("insert-pipe"),    PipePointType::Run);
    EXPECT_EQ(InsertComponentCommand::mapComponentType("insert-elbow"),   PipePointType::Bend);
    EXPECT_EQ(InsertComponentCommand::mapComponentType("insert-tee"),     PipePointType::Tee);
    EXPECT_EQ(InsertComponentCommand::mapComponentType("insert-reducer"), PipePointType::Reducer);
    EXPECT_EQ(InsertComponentCommand::mapComponentType("insert-valve"),   PipePointType::Valve);
}

TEST(InsertComponentMapping, UnknownTypeThrows)
{
    EXPECT_THROW(
        command::InsertComponentCommand::mapComponentType("insert-unknown"),
        std::runtime_error);
}

// ============================================================
// 基本插入 + execute/undo/redo
// ============================================================

TEST_F(InsertComponentTest, InsertPipeCreatesRunPoint)
{
    auto cmd = command::InsertComponentCommand::create(
        "insert-pipe", routeId, segId, 100.0, 200.0, 300.0);

    EXPECT_EQ(cmd->componentType(), "insert-pipe");
    EXPECT_EQ(cmd->type(), command::CommandType::Macro);

    cmd->execute(ctx);
    EXPECT_TRUE(cmd->lastResult().success);
    EXPECT_EQ(cmd->lastResult().createdIds.size(), 1u);

    // 验证管点已创建
    auto points = doc->allPipePoints();
    ASSERT_EQ(points.size(), 1u);
    EXPECT_EQ(points[0]->type(), model::PipePointType::Run);
    EXPECT_DOUBLE_EQ(points[0]->position().X(), 100.0);
    EXPECT_DOUBLE_EQ(points[0]->position().Y(), 200.0);
    EXPECT_DOUBLE_EQ(points[0]->position().Z(), 300.0);

    // undo → 管点被删除
    cmd->undo(ctx);
    EXPECT_TRUE(doc->allPipePoints().empty());

    // redo → 管点恢复
    cmd->execute(ctx);
    EXPECT_EQ(doc->allPipePoints().size(), 1u);
}

TEST_F(InsertComponentTest, InsertElbowCreatesBendPoint)
{
    auto cmd = command::InsertComponentCommand::create(
        "insert-elbow", routeId, segId, 50.0, 60.0, 70.0);

    cmd->execute(ctx);
    EXPECT_TRUE(cmd->lastResult().success);

    auto points = doc->allPipePoints();
    ASSERT_EQ(points.size(), 1u);
    EXPECT_EQ(points[0]->type(), model::PipePointType::Bend);
}

TEST_F(InsertComponentTest, InsertTeeCreatesBranchSegment)
{
    auto cmd = command::InsertComponentCommand::create(
        "insert-tee", routeId, segId, 10.0, 20.0, 30.0);

    cmd->execute(ctx);
    EXPECT_TRUE(cmd->lastResult().success);

    auto points = doc->allPipePoints();
    ASSERT_EQ(points.size(), 1u);
    EXPECT_EQ(points[0]->type(), model::PipePointType::Tee);

    // Tee 应创建分支段
    EXPECT_GT(route->segmentCount(), 1u);

    // undo → 分支段和管点都应清除
    cmd->undo(ctx);
    EXPECT_TRUE(doc->allPipePoints().empty());
    EXPECT_EQ(route->segmentCount(), 1u);
}

TEST_F(InsertComponentTest, InsertReducerCreatesReducerPoint)
{
    auto cmd = command::InsertComponentCommand::create(
        "insert-reducer", routeId, segId, 0.0, 0.0, 0.0);

    cmd->execute(ctx);
    ASSERT_EQ(doc->allPipePoints().size(), 1u);
    EXPECT_EQ(doc->allPipePoints()[0]->type(), model::PipePointType::Reducer);
}

TEST_F(InsertComponentTest, InsertValveCreatesValvePoint)
{
    auto cmd = command::InsertComponentCommand::create(
        "insert-valve", routeId, segId, 0.0, 0.0, 0.0);

    cmd->execute(ctx);
    ASSERT_EQ(doc->allPipePoints().size(), 1u);
    EXPECT_EQ(doc->allPipePoints()[0]->type(), model::PipePointType::Valve);
}

// ============================================================
// JSON 序列化 / 反序列化
// ============================================================

TEST_F(InsertComponentTest, JsonRoundTrip)
{
    auto cmd = command::InsertComponentCommand::create(
        "insert-tee", routeId, segId, 11.0, 22.0, 33.0);

    cmd->execute(ctx);

    // 序列化
    auto j = cmd->toJson();
    EXPECT_EQ(j.at("type").get<std::string>(), "InsertComponent");
    EXPECT_EQ(j.at("componentType").get<std::string>(), "insert-tee");
    EXPECT_TRUE(j.contains("children"));
    EXPECT_EQ(j.at("children").size(), 1u);

    // 反序列化
    auto restored = command::InsertComponentCommand::fromJson(j);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->componentType(), "insert-tee");

    // 验证子命令存在
    EXPECT_EQ(restored->children().size(), 1u);
}

TEST_F(InsertComponentTest, RegistryCanDeserialize)
{
    command::CommandRegistry registry;
    registry.registerBuiltins();

    auto cmd = command::InsertComponentCommand::create(
        "insert-pipe", routeId, segId, 1.0, 2.0, 3.0);
    cmd->execute(ctx);

    auto j = cmd->toJson();

    // 通过 CommandRegistry 反序列化
    auto restored = registry.deserialize(j);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->type(), command::CommandType::Macro);
}

// ============================================================
// CommandStack 集成
// ============================================================

TEST_F(InsertComponentTest, CommandStackUndoRedo)
{
    command::CommandStack stack;

    auto cmd = command::InsertComponentCommand::create(
        "insert-elbow", routeId, segId, 100.0, 0.0, 0.0);
    stack.execute(std::move(cmd), ctx);

    EXPECT_EQ(doc->allPipePoints().size(), 1u);
    EXPECT_TRUE(stack.canUndo());
    EXPECT_FALSE(stack.canRedo());

    stack.undo(ctx);
    EXPECT_TRUE(doc->allPipePoints().empty());
    EXPECT_TRUE(stack.canRedo());

    stack.redo(ctx);
    EXPECT_EQ(doc->allPipePoints().size(), 1u);
}

// ============================================================
// deleteSelected 迁移验证（通过命令栈执行 DeletePipePointCommand）
// ============================================================

TEST_F(InsertComponentTest, DeleteViaCommandStack)
{
    command::CommandStack stack;

    // 插入两个管点
    auto cmd1 = command::InsertComponentCommand::create(
        "insert-pipe", routeId, segId, 0.0, 0.0, 0.0);
    stack.execute(std::move(cmd1), ctx);

    auto cmd2 = command::InsertComponentCommand::create(
        "insert-pipe", routeId, segId, 100.0, 0.0, 0.0);
    stack.execute(std::move(cmd2), ctx);

    ASSERT_EQ(doc->allPipePoints().size(), 2u);

    // 删除第一个管点
    auto ppId = doc->allPipePoints()[0]->id();
    auto delCmd = command::DeletePipePointCommand::create(ppId);
    stack.execute(std::move(delCmd), ctx);

    EXPECT_EQ(doc->allPipePoints().size(), 1u);

    // undo 恢复
    stack.undo(ctx);
    EXPECT_EQ(doc->allPipePoints().size(), 2u);
}
