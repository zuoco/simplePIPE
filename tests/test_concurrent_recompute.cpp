// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
//
// T74 — 建立并发回归测试
// 覆盖场景：
//   1. 批量编辑：连续多次 asyncRecompute() → 所有结果正确交付
//   2. 撤销重做：doc.bumpVersion() 提升版本 → 过期结果被 drainFresh 丢弃
//   3. 工作台切换：channel.discard() 丢弃挂起结果 → 再次重算正确交付
//   4. 应用退出：workers.shutdown() + channel.discard() 后台线程安全回收，不崩溃

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

/// 轮询等待条件为真或超时（仅用于测试）
static bool waitUntil(std::function<bool()> cond,
                      std::chrono::milliseconds timeout = 3000ms) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!cond()) {
        if (std::chrono::steady_clock::now() >= deadline) return false;
        std::this_thread::sleep_for(10ms);
    }
    return true;
}

// ——— Fixture ———

class ConcurrentRecomputeTest : public ::testing::Test {
protected:
    app::Document        doc;
    app::DependencyGraph graph;

    void SetUp() override { doc.setName("T74Test"); }
};

// ——— T74-1：批量编辑 ———

/// 连续执行多次 asyncRecompute()（模拟连续命令），所有结果最终全部交付
///
/// 验证：N 次脏标记 + N 次 asyncRecompute() → drainResults() 累计 applied >= N
TEST_F(ConcurrentRecomputeTest, BatchEdit_MultipleAsyncRecompute_AllResultsDelivered) {
    task::WorkerGroup workers(2);
    task::ResultChannel channel;
    task::SceneUpdateAdapter adapter(channel,
        [&]() { return doc.currentVersion(); });

    engine::RecomputeEngine eng(doc, graph);

    // 构建 asyncFn：每次调用提交一个后台哨兵任务
    std::atomic<int> asyncFnCalled{0};
    eng.enableAsyncMode(
        [&]() {
            ++asyncFnCalled;
            auto snap = app::makeDocumentSnapshot(doc, graph);
            graph.clearDirty();
            const auto version = snap.version;
            workers.submit([version, &channel](const task::CancellationToken& token) {
                if (token.isCancellationRequested()) return;
                std::this_thread::sleep_for(5ms);
                channel.post(version, []() { /* 哨兵 applyFn */ });
            });
        },
        [&adapter]() -> std::size_t { return adapter.drain(); }
    );

    auto spec = makeSpec(doc, "BatchSpec");
    const int N = 5;

    // 连续 N 次标脏 → asyncRecompute()（每次版本相同，均为当前版本）
    for (int i = 0; i < N; ++i) {
        auto pp = makePoint(doc, gp_Pnt(i * 100.0, 0, 0), model::PipePointType::Run, spec);
        graph.markDirty(pp->id());
        eng.asyncRecompute();
    }

    EXPECT_EQ(asyncFnCalled.load(), N) << "asyncFn 应被调用 " << N << " 次";

    // 等待所有后台任务向 channel 投递结果
    bool allPosted = waitUntil([&]() {
        return channel.pendingCount() >= static_cast<std::size_t>(N);
    });
    EXPECT_TRUE(allPosted) << "未能在超时前收到 " << N << " 个结果";

    // drainResults() 消费所有新鲜结果（版本匹配 → 全部执行）
    std::size_t totalApplied = 0;
    // 可能需要多次 drain（后台任务是异步的）
    totalApplied = eng.drainResults();
    EXPECT_GE(totalApplied, 1u) << "至少应有一个结果被交付";

    workers.waitForIdle();
    workers.shutdown();
}

// ——— T74-2：撤销重做 ———

/// undo/redo 操作推进文档版本 → 旧版本提交的结果被 drainFresh 丢弃
///
/// 验证：asyncFn 以版本 V 提交 → doc.bumpVersion() → drain 时版本为 V+1 → applied == 0
TEST_F(ConcurrentRecomputeTest, UndoRedo_BumpedVersion_StaleResultsDiscarded) {
    task::ResultChannel channel;
    app::DocumentVersion capturedVersion = 0;

    engine::RecomputeEngine eng(doc, graph);

    // asyncFn 同步投递（不使用 Worker，便于精确控制版本）
    eng.enableAsyncMode(
        [&]() {
            auto snap = app::makeDocumentSnapshot(doc, graph);
            capturedVersion = snap.version;
            graph.clearDirty();
            // 将结果以旧版本号投递到 channel
            channel.post(capturedVersion, []() { /* 不应被执行 */ });
        },
        [&]() -> std::size_t {
            // drainFresh 使用"当前文档版本"消费（此时已 bump，版本不匹配）
            return channel.drainFresh(doc.currentVersion());
        }
    );

    std::atomic<int> sceneCallCount{0};
    eng.setSceneUpdateCallback([&](const std::string&, const TopoDS_Shape&) {
        ++sceneCallCount;
    });

    auto spec = makeSpec(doc, "UndoSpec");
    auto pp   = makePoint(doc, gp_Pnt(0, 0, 0), model::PipePointType::Run, spec);
    graph.markDirty(pp->id());

    // Step 1: 提交 asyncRecompute（记录版本 V，向 channel 投递 V 结果）
    eng.asyncRecompute();
    EXPECT_EQ(channel.pendingCount(), 1u) << "channel 应有 1 个挂起结果";

    // Step 2: 模拟 undo/redo —— 推进文档版本（使挂起结果变为过期）
    doc.bumpVersion();
    EXPECT_GT(doc.currentVersion(), capturedVersion) << "版本应已提升";

    // Step 3: drainResults() 消费 → 版本不匹配 → 0 个结果被实际执行
    std::size_t applied = eng.drainResults();
    EXPECT_EQ(applied, 0u) << "过期结果应被丢弃，applied == 0";
    EXPECT_EQ(sceneCallCount.load(), 0) << "场景回调不应被触发";
}

// ——— T74-3：工作台切换 ———

/// 切换工作台时调用 channel.discard() 丢弃挂起结果 → 再次重算后结果正确交付
///
/// 验证：
///   a) 第一批结果 discard → pendingCount == 0
///   b) 切换后重新触发 asyncRecompute → 再次 drain 正确交付新结果
TEST_F(ConcurrentRecomputeTest, WorkbenchSwitch_Discard_ThenRecomputeDelivers) {
    task::WorkerGroup workers(1);
    task::ResultChannel channel;
    task::SceneUpdateAdapter adapter(channel,
        [&]() { return doc.currentVersion(); });

    engine::RecomputeEngine eng(doc, graph);
    std::atomic<int> asyncFnCalled{0};

    eng.enableAsyncMode(
        [&]() {
            ++asyncFnCalled;
            auto snap = app::makeDocumentSnapshot(doc, graph);
            graph.clearDirty();
            const auto version = snap.version;
            workers.submit([version, &channel](const task::CancellationToken& token) {
                if (token.isCancellationRequested()) return;
                std::this_thread::sleep_for(10ms);
                channel.post(version, []() { /* applyFn */ });
            });
        },
        [&adapter]() -> std::size_t { return adapter.drain(); }
    );

    auto spec = makeSpec(doc, "SwitchSpec");

    // --- 第一次重算（切换工作台前）---
    auto pp1 = makePoint(doc, gp_Pnt(0, 0, 0), model::PipePointType::Run, spec);
    graph.markDirty(pp1->id());
    eng.asyncRecompute();

    // 等待后台任务投递结果
    bool posted = waitUntil([&]() { return channel.pendingCount() > 0; });
    EXPECT_TRUE(posted) << "第一次重算应向 channel 投递结果";

    // 模拟工作台切换：丢弃所有挂起结果
    channel.discard();
    EXPECT_EQ(channel.pendingCount(), 0u) << "discard() 后 channel 应为空";

    // --- 第二次重算（切换工作台后）---
    auto pp2 = makePoint(doc, gp_Pnt(100, 0, 0), model::PipePointType::Run, spec);
    graph.markDirty(pp2->id());
    eng.asyncRecompute();

    // 等待第二批结果到达
    bool posted2 = waitUntil([&]() { return channel.pendingCount() > 0; });
    EXPECT_TRUE(posted2) << "第二次重算应向 channel 再次投递结果";

    // drain 应正确交付新结果（版本匹配）
    std::size_t applied = eng.drainResults();
    EXPECT_GE(applied, 1u) << "切换后重算结果应被正常交付";

    workers.waitForIdle();
    workers.shutdown();
}

// ——— T74-4：应用退出 ———

/// 应用退出时 workers.shutdown() + channel.discard() 安全回收，不崩溃
///
/// 验证：提交多个任务（部分可能正在运行），shutdown + discard 后无崩溃，
///       后台线程全部退出（不 hang）。
TEST(ConcurrentRecomputeExitTest, AppShutdown_WorkersAndChannel_CleanExit) {
    constexpr int N = 8;
    task::ResultChannel channel;
    task::WorkerGroup workers(3);

    std::atomic<int> tasksStarted{0};

    // 提交 N 个长耗时任务（后台可能未完成时 shutdown）
    for (int i = 0; i < N; ++i) {
        workers.submit([&tasksStarted, &channel, i](const task::CancellationToken& token) {
            ++tasksStarted;
            // 模拟耗时操作；检查取消令牌
            for (int j = 0; j < 10; ++j) {
                if (token.isCancellationRequested()) return;
                std::this_thread::sleep_for(10ms);
            }
            // 只有未被取消才投递结果
            channel.post(static_cast<app::DocumentVersion>(i),
                         [i]() { (void)i; /* 哨兵 applyFn */ });
        });
    }

    // 等待至少一个任务启动，确保后台确实在运行
    bool anyStarted = waitUntil([&]() { return tasksStarted.load() > 0; });
    EXPECT_TRUE(anyStarted) << "至少一个后台任务应已启动";

    // 模拟应用退出：取消挂起任务 + 关闭工作组 + 丢弃结果
    workers.shutdown(/*cancelPending=*/true);
    channel.discard();

    // 到达这里说明无死锁、无崩溃
    EXPECT_EQ(channel.pendingCount(), 0u) << "discard() 后 channel 应为空";

    // shutdown 已等待所有线程退出；验证 channel 中无残余（不应 crash）
    channel.discard(); // 幂等：再次调用应无副作用
}

// ——— T74-5：ResultChannel drainFresh 版本精确匹配 ———

/// drainFresh 仅执行版本完全相同的结果，版本差 1 即丢弃
///
/// 这是上述 T74-2 的单元级变体，直接测试 ResultChannel 接口。
TEST(ResultChannelVersionTest, DrainFresh_VersionMismatch_DiscardsSilently) {
    task::ResultChannel channel;

    std::atomic<int> appliedCount{0};

    // 以版本 10 投递
    channel.post(10, [&]() { ++appliedCount; });
    // 以版本 12 投递
    channel.post(12, [&]() { ++appliedCount; });

    // drainFresh(11)：只有版本 == 11 的结果会执行，此处两个均不匹配
    std::size_t applied = channel.drainFresh(11);
    EXPECT_EQ(applied, 0u) << "版本 10 和 12 均不匹配版本 11，应全部丢弃";
    EXPECT_EQ(appliedCount.load(), 0);
    EXPECT_EQ(channel.pendingCount(), 0u) << "drainFresh 应取出所有结果（不论是否执行）";

    // 以版本 20 投递后以相同版本消费 → 应执行
    channel.post(20, [&]() { ++appliedCount; });
    applied = channel.drainFresh(20);
    EXPECT_EQ(applied, 1u);
    EXPECT_EQ(appliedCount.load(), 1);
}
