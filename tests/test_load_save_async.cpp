// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
//
// T73 — 后台化加载恢复与保存前准备 单元测试
// 覆盖场景：
//   1. asyncRecomputeAll() 无异步模式时退化为同步 recomputeAll()（场景回调被调用）
//   2. asyncRecomputeAll() 有异步模式时将所有 PipePoint 标记脏 → asyncFn 被调用
//   3. 加载恢复模拟：Document 含多段 PipePoint，asyncRecomputeAll() 提交等量后台任务
//   4. 保存前排空：drainResults() 确保所有挂起结果在"保存"前已提交完毕

#include "engine/RecomputeEngine.h"
#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "app/DocumentSnapshot.h"
#include "lib/runtime/task/TaskQueue.h"
#include "lib/runtime/task/ResultChannel.h"
#include "lib/runtime/task/SceneUpdateAdapter.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Segment.h"
#include "model/Route.h"

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// ——— 辅助函数 ———

static std::shared_ptr<model::PipeSpec> makeSpec(app::Document& doc,
                                                  const std::string& name,
                                                  double od = 50.0,
                                                  double wt = 5.0) {
    auto spec = std::make_shared<model::PipeSpec>(name);
    spec->setOd(od);
    spec->setWallThickness(wt);
    doc.addObject(spec);
    return spec;
}

/// 构建含 N 个 PipePoint 的 Route+Segment 并注入 Document
static void buildRoute(app::Document& doc,
                       const std::string& routeName,
                       int pointCount,
                       std::shared_ptr<model::PipeSpec> spec) {
    auto route = std::make_shared<model::Route>(routeName);
    auto seg   = std::make_shared<model::Segment>("Seg1");
    for (int i = 0; i < pointCount; ++i) {
        auto pp = std::make_shared<model::PipePoint>();
        pp->setPosition(gp_Pnt(i * 100.0, 0, 0));
        pp->setType(model::PipePointType::Run);
        if (spec) pp->setPipeSpec(spec);
        seg->addPoint(pp);
        doc.addObject(pp);
    }
    route->addSegment(seg);
    // Segment 必须注册到 Document，使 allSegments() 可以发现它
    doc.addObject(seg);
    doc.addObject(route);
}

/// 等待条件为真或超时
static bool waitUntil(std::function<bool()> cond,
                      std::chrono::milliseconds timeout = 2000ms) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!cond()) {
        if (std::chrono::steady_clock::now() >= deadline) return false;
        std::this_thread::sleep_for(10ms);
    }
    return true;
}

// ——— Fixture ———

class LoadSaveAsyncTest : public ::testing::Test {
protected:
    app::Document        doc;
    app::DependencyGraph graph;

    void SetUp() override { doc.setName("T73Test"); }
};

// ——— T73-1：无异步模式时退化为同步 recomputeAll ———

/// asyncRecomputeAll() 在无异步模式时通过场景回调同步交付所有点
TEST_F(LoadSaveAsyncTest, AsyncRecomputeAll_SyncFallback_WhenNoAsyncMode) {
    auto spec = makeSpec(doc, "S1");
    buildRoute(doc, "R1", 3, spec);

    engine::RecomputeEngine eng(doc, graph);

    std::atomic<int> cbCount{0};
    eng.setSceneUpdateCallback([&](const std::string&, const TopoDS_Shape& shape) {
        if (!shape.IsNull()) ++cbCount;
    });

    // 未启用异步模式 → 退化为同步全量重算
    eng.asyncRecomputeAll();

    // 同步路径：所有点的几何应在本调用内完成
    EXPECT_EQ(cbCount.load(), 3);
}

// ——— T73-2：异步模式下 asyncRecomputeAll 触发 asyncFn ———

/// asyncRecomputeAll() 在有异步模式时将所有 PipePoint 标记脏并调用 asyncFn
TEST_F(LoadSaveAsyncTest, AsyncRecomputeAll_CallsAsyncFn_WhenAsyncModeEnabled) {
    auto spec = makeSpec(doc, "S2");
    buildRoute(doc, "R2", 4, spec);

    engine::RecomputeEngine eng(doc, graph);

    std::atomic<int> asyncFnCallCount{0};
    std::size_t dirtyCountInAsyncFn = 0;

    eng.enableAsyncMode(
        [&]() {
            ++asyncFnCallCount;
            // 在 asyncFn 内部捕获脏集合大小，验证所有点都被标记
            auto dirty = graph.collectDirty();
            dirtyCountInAsyncFn = dirty.size();
            graph.clearDirty();
        },
        []() -> std::size_t { return 0; }
    );

    eng.asyncRecomputeAll();

    // asyncFn 应被调用一次
    EXPECT_EQ(asyncFnCallCount.load(), 1);
    // 脏集合应包含所有 4 个 PipePoint
    EXPECT_EQ(dirtyCountInAsyncFn, 4u);
}

// ——— T73-3：加载恢复模拟 ———

/// 模拟工程加载后恢复几何：Document 含多段多路由，asyncRecomputeAll()
/// 提交后台任务，ResultChannel 收到等量 post
TEST_F(LoadSaveAsyncTest, LoadRecovery_AsyncRecomputeAll_SubmitsTasksForAllPoints) {
    auto spec = makeSpec(doc, "S3");
    // 模拟加载后的文档：2 条路由，各 3 个点，共 6 个点
    buildRoute(doc, "R3a", 3, spec);
    buildRoute(doc, "R3b", 3, spec);

    engine::RecomputeEngine eng(doc, graph);

    task::ResultChannel channel;
    std::atomic<int> postCount{0};

    eng.enableAsyncMode(
        [&]() {
            // 简化版 asyncFn：按脏 ID 向 channel 各 post 一条记录
            auto snap = app::makeDocumentSnapshot(doc, graph);
            graph.clearDirty();
            const auto ver = snap.version;
            for (const auto& id : snap.dependencyGraph.dirtyIds) {
                (void)id;
                channel.post(ver, [&postCount]() { ++postCount; });
            }
        },
        [&]() -> std::size_t {
            return channel.drainFresh(doc.currentVersion());
        }
    );

    eng.asyncRecomputeAll();

    // 所有 6 个点都应收到一条结果
    EXPECT_EQ(channel.pendingCount(), 6u);

    // drain 消费全部结果
    std::size_t drained = eng.drainResults();
    EXPECT_EQ(drained, 6u);
    EXPECT_EQ(postCount.load(), 6);
}

// ——— T73-4：保存前排空（同步 drain）———

/// 保存前调用 drainResults() 确保后台几何结果全部交付
/// 模拟场景：asyncFn 将结果同步写入 channel，drain 后 pendingCount == 0
TEST_F(LoadSaveAsyncTest, PreSave_DrainResults_FlushesAllPendingResults) {
    auto spec = makeSpec(doc, "S4");
    buildRoute(doc, "R4", 5, spec);

    engine::RecomputeEngine eng(doc, graph);

    task::ResultChannel channel;
    std::atomic<int> appliedCount{0};

    eng.enableAsyncMode(
        [&]() {
            auto snap = app::makeDocumentSnapshot(doc, graph);
            graph.clearDirty();
            const auto ver = snap.version;
            for (const auto& id : snap.dependencyGraph.dirtyIds) {
                (void)id;
                channel.post(ver, [&appliedCount]() { ++appliedCount; });
            }
        },
        [&]() -> std::size_t {
            return channel.drainFresh(doc.currentVersion());
        }
    );

    eng.asyncRecomputeAll();

    // 保存前排空：此时后台任务已同步投递（简化测试环境无真实后台线程）
    EXPECT_GT(channel.pendingCount(), 0u);

    // 排空所有挂起结果（保存前准备）
    std::size_t flushed = eng.drainResults();
    EXPECT_EQ(flushed, 5u);
    EXPECT_EQ(appliedCount.load(), 5);
    EXPECT_EQ(channel.pendingCount(), 0u);
}
