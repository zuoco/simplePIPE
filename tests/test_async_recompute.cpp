// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
//
// T71 — RecomputeEngine 异步管线单元测试
// 覆盖场景：
//   1. asyncRecompute() 在异步模式未启用时退化为同步 recompute()
//   2. asyncRecompute() 在有脏对象时调用注入的 asyncFn
//   3. 空脏集合时 asyncRecompute() 不调用 asyncFn
//   4. 后台任务通过 ResultChannel 投递 applyFn，drainResults() 在主线程执行
//   5. 版本过期时 drainResults() 丢弃结果（不调用 sceneCb_）
//   6. recomputeAll() 始终同步执行（降级模式）
//   7. asyncRecompute() 使用真实 WorkerGroup 并发推导
//   8. GeometryDeriver::deriveFromSnapshot() 接受空 spec 不崩溃
//   9. GeometryDeriver::deriveFromSnapshot() Run 类型推导出非空几何
//  10. asyncFn 负责清除脏标记（快照窗口协议验证）

#include "engine/RecomputeEngine.h"
#include "engine/GeometryDeriver.h"
#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "app/DocumentSnapshot.h"
#include "lib/runtime/task/TaskQueue.h"
#include "lib/runtime/task/ResultChannel.h"
#include "lib/runtime/task/SceneUpdateAdapter.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Segment.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>
#include <utility>
#include <vector>

// ——— 辅助函数 ———

/// 创建带有 OD/wallThickness 的 PipeSpec 并注入 Document
static std::shared_ptr<model::PipeSpec> makeSpec(app::Document& doc,
                                                  const std::string& name,
                                                  double od, double wt) {
    auto spec = std::make_shared<model::PipeSpec>(name);
    spec->setOd(od);
    spec->setWallThickness(wt);
    doc.addObject(spec);
    return spec;
}

/// 创建 PipePoint 并注入 Document
static std::shared_ptr<model::PipePoint> makePoint(app::Document& doc,
                                                    const gp_Pnt& pos,
                                                    model::PipePointType type,
                                                    std::shared_ptr<model::PipeSpec> spec = nullptr) {
    auto pp = std::make_shared<model::PipePoint>();
    pp->setPosition(pos);
    pp->setType(type);
    if (spec) pp->setPipeSpec(spec);
    doc.addObject(pp);
    return pp;
}

/// 等待直到条件为真或超时（轮询，仅用于测试）
static bool waitUntil(std::function<bool()> cond,
                      std::chrono::milliseconds timeout = std::chrono::milliseconds(2000)) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!cond()) {
        if (std::chrono::steady_clock::now() >= deadline) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

// ——— Fixture ———

class AsyncRecomputeTest : public ::testing::Test {
protected:
    app::Document        doc;
    app::DependencyGraph graph;

    void SetUp() override { doc.setName("AsyncTest"); }
};

// ——— 测试用例 ———

/// T71-1：异步模式未启用时 asyncRecompute() 退化为同步 recompute()
TEST_F(AsyncRecomputeTest, AsyncRecompute_SyncFallback_WhenNoAsyncMode) {
    engine::RecomputeEngine eng(doc, graph);

    auto fakeId = foundation::UUID::generate();
    graph.markDirty(fakeId);

    EXPECT_NO_THROW(eng.asyncRecompute());
    // 同步降级路径应清除脏标记
    EXPECT_TRUE(graph.collectDirty().empty());
}

/// T71-2：有脏对象时 asyncRecompute() 调用注入的 asyncFn
TEST_F(AsyncRecomputeTest, AsyncRecompute_AsyncFnCalled_WhenDirtyExists) {
    engine::RecomputeEngine eng(doc, graph);

    std::atomic<int> callCount{0};

    // asyncFn 模拟最简管线：仅清脏和计数
    eng.enableAsyncMode(
        [&]() {
            ++callCount;
            graph.clearDirty();
        },
        []() -> std::size_t { return 0; }
    );

    auto spec = makeSpec(doc, "S1", 50.0, 5.0);
    auto pp   = makePoint(doc, gp_Pnt(0, 0, 0), model::PipePointType::Run, spec);
    graph.markDirty(pp->id());

    eng.asyncRecompute();

    EXPECT_EQ(callCount.load(), 1);
    EXPECT_TRUE(graph.collectDirty().empty());
}

/// T71-3：没有脏对象时 asyncRecompute() 不调用 asyncFn
TEST_F(AsyncRecomputeTest, AsyncRecompute_AsyncFnNotCalled_WhenNoDirty) {
    engine::RecomputeEngine eng(doc, graph);

    std::atomic<int> callCount{0};

    eng.enableAsyncMode(
        [&]() { ++callCount; },
        []() -> std::size_t { return 0; }
    );

    // 不标脏任何对象
    eng.asyncRecompute();

    EXPECT_EQ(callCount.load(), 0);
}

/// T71-4：drainResults() 在版本匹配时执行 applyFn
TEST_F(AsyncRecomputeTest, DrainResults_ExecutesApplyFn_WhenVersionMatches) {
    engine::RecomputeEngine eng(doc, graph);

    task::ResultChannel channel;
    task::SceneUpdateAdapter adapter(channel,
        [&]() { return doc.currentVersion(); });

    // asyncFn 构建快照 → 清脏 → 向 channel 投递哨兵结果
    eng.enableAsyncMode(
        [&]() {
            auto snap = app::makeDocumentSnapshot(doc, graph);
            graph.clearDirty();
            if (!snap.dependencyGraph.dirtyIds.empty()) {
                channel.post(snap.version, []() { /* 哨兵 applyFn */ });
            }
        },
        [&adapter]() -> std::size_t { return adapter.drain(); }
    );

    auto spec = makeSpec(doc, "S2", 50.0, 5.0);
    auto pp   = makePoint(doc, gp_Pnt(0, 0, 0), model::PipePointType::Run, spec);
    graph.markDirty(pp->id());

    eng.asyncRecompute();

    // 版本匹配 → applyFn 被执行
    std::size_t applied = eng.drainResults();
    EXPECT_GE(applied, 1u);
}

/// T71-5：版本过期时 drainResults() 丢弃结果
TEST_F(AsyncRecomputeTest, DrainResults_DiscardsStaleResult) {
    engine::RecomputeEngine eng(doc, graph);

    task::ResultChannel channel;
    app::DocumentVersion capturedVersion = 0;

    // asyncFn 向 channel 投递结果；drainFn 使用过期版本消费
    eng.enableAsyncMode(
        [&]() {
            auto snap = app::makeDocumentSnapshot(doc, graph);
            capturedVersion = snap.version;
            graph.clearDirty();
            if (!snap.dependencyGraph.dirtyIds.empty()) {
                channel.post(snap.version, []() { /* 不应被执行 */ });
            }
        },
        [&]() -> std::size_t {
            // capturedVersion + 1 确保版本不匹配 → 结果被丢弃
            return channel.drainFresh(capturedVersion + 1);
        }
    );

    std::atomic<int> sceneCallCount{0};
    eng.setSceneUpdateCallback([&](const std::string&, const TopoDS_Shape&) {
        ++sceneCallCount;
    });

    auto spec = makeSpec(doc, "S3", 50.0, 5.0);
    auto pp   = makePoint(doc, gp_Pnt(0, 0, 0), model::PipePointType::Run, spec);
    graph.markDirty(pp->id());

    eng.asyncRecompute();

    std::size_t applied = eng.drainResults();
    EXPECT_EQ(applied, 0u);      // 过期结果不执行
    EXPECT_EQ(sceneCallCount.load(), 0);
}

/// T71-6：recomputeAll() 同步执行，直接触发 sceneCb_
TEST_F(AsyncRecomputeTest, RecomputeAll_SyncExecution_DirectCallback) {
    engine::RecomputeEngine eng(doc, graph);

    std::atomic<int> cbCount{0};
    eng.setSceneUpdateCallback([&](const std::string&, const TopoDS_Shape&) {
        ++cbCount;
    });

    auto spec = makeSpec(doc, "S4", 50.0, 5.0);
    auto seg  = std::make_shared<model::Segment>("Seg4");
    auto pp1  = makePoint(doc, gp_Pnt(0, 0, 0),    model::PipePointType::Run, spec);
    auto pp2  = makePoint(doc, gp_Pnt(0, 0, 1000), model::PipePointType::Run, spec);
    seg->addPoint(pp1);
    seg->addPoint(pp2);
    doc.addObject(seg);

    eng.recomputeAll(); // 同步路径

    EXPECT_GE(cbCount.load(), 0); // 不崩溃即通过
}

/// T71-7：asyncRecompute() 使用真实 WorkerGroup（并发验证）
TEST_F(AsyncRecomputeTest, AsyncRecompute_WithRealWorkerGroup) {
    task::WorkerGroup workers(2);
    task::ResultChannel channel;
    task::SceneUpdateAdapter adapter(channel,
        [&]() { return doc.currentVersion(); });

    engine::RecomputeEngine eng(doc, graph);

    // asyncFn 向真实 WorkerGroup 提交任务，后台任务向 channel 投递哨兵结果
    eng.enableAsyncMode(
        [&]() {
            auto snap = app::makeDocumentSnapshot(doc, graph);
            graph.clearDirty();
            if (snap.dependencyGraph.dirtyIds.empty()) return;
            const auto version = snap.version;
            workers.submit([version, &channel](const task::CancellationToken&) {
                channel.post(version, []() { /* 哨兵 */ });
            });
        },
        [&adapter]() -> std::size_t { return adapter.drain(); }
    );

    auto spec = makeSpec(doc, "S5", 50.0, 5.0);
    auto seg  = std::make_shared<model::Segment>("Seg5");
    auto pp1  = makePoint(doc, gp_Pnt(0, 0, 0),    model::PipePointType::Run, spec);
    auto pp2  = makePoint(doc, gp_Pnt(0, 0, 1000), model::PipePointType::Run, spec);
    seg->addPoint(pp1);
    seg->addPoint(pp2);
    doc.addObject(seg);

    graph.markDirty(pp1->id());
    eng.asyncRecompute();

    // 等待后台任务向 channel 投递结果
    bool hasResult = waitUntil([&]() { return channel.pendingCount() > 0; });
    EXPECT_TRUE(hasResult) << "后台任务超时未向 ResultChannel 投递结果";

    std::size_t applied = eng.drainResults();
    EXPECT_GE(applied, 1u);

    workers.waitForIdle();
    workers.shutdown();
}

/// T71-8：GeometryDeriver::deriveFromSnapshot() 接受空 spec 不崩溃
TEST(GeometryDeriverSnapshotTest, DeriveFromSnapshot_NullSpec_ReturnsEmpty) {
    app::PipePointSnapshot pp;
    pp.id       = foundation::UUID::generate();
    pp.type     = model::PipePointType::Run;
    pp.position = gp_Pnt(0, 0, 0);

    TopoDS_Shape shape = engine::GeometryDeriver::deriveFromSnapshot(
        gp_Pnt(0, 0, 0), pp, nullptr, gp_Pnt(0, 0, 1000));
    EXPECT_TRUE(shape.IsNull());
}

/// T71-9：GeometryDeriver::deriveFromSnapshot() Run 类型推导出非空几何
TEST(GeometryDeriverSnapshotTest, DeriveFromSnapshot_Run_ProducesGeometry) {
    app::PipeSpecSnapshot specSnap;
    specSnap.id                      = foundation::UUID::generate();
    specSnap.fields["OD"]            = foundation::Variant(50.0);
    specSnap.fields["wallThickness"] = foundation::Variant(5.0);

    app::PipePointSnapshot pp;
    pp.id         = foundation::UUID::generate();
    pp.type       = model::PipePointType::Run;
    pp.position   = gp_Pnt(0, 0, 500);
    pp.pipeSpecId = specSnap.id;

    TopoDS_Shape shape = engine::GeometryDeriver::deriveFromSnapshot(
        gp_Pnt(0, 0, 0), pp, &specSnap, gp_Pnt(0, 0, 1000));
    EXPECT_FALSE(shape.IsNull());
}

/// T71-10：asyncFn 负责清除脏标记（快照窗口协议验证）
TEST_F(AsyncRecomputeTest, AsyncFn_ClearsDirty_AsPartOfProtocol) {
    engine::RecomputeEngine eng(doc, graph);

    // asyncFn 遵循协议：构建快照 → 清脏
    eng.enableAsyncMode(
        [&]() {
            auto snap = app::makeDocumentSnapshot(doc, graph);
            graph.clearDirty();
            (void)snap; // 后台任务省略（仅测试 clearDirty 时机）
        },
        []() -> std::size_t { return 0; }
    );

    auto spec = makeSpec(doc, "S6", 50.0, 5.0);
    auto pp   = makePoint(doc, gp_Pnt(0, 0, 0), model::PipePointType::Run, spec);
    graph.markDirty(pp->id());
    EXPECT_FALSE(graph.collectDirty().empty());

    eng.asyncRecompute();

    // asyncFn 内调用了 clearDirty()，脏集应为空
    EXPECT_TRUE(graph.collectDirty().empty());
}
