// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0
//
// T72 — 后台化 ShapeMesher 与批量重算 单元测试
// 覆盖场景：
//   1. 多个脏对象 → 每对象独立提交任务 → ResultChannel 收到等量 post
//   2. WorkerGroup(2) 可并发执行两个任务（批量不串行）
//   3. ShapeMesher::mesh() 在后台线程中可正常调用并返回非空结果
//   4. 批量并行任务全部完成后 drain 正确交付所有结果，不遗漏

#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "app/DocumentSnapshot.h"
#include "geometry/ShapeMesher.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Segment.h"
#include "task/ResultChannel.h"
#include "task/SceneUpdateAdapter.h"
#include "task/TaskQueue.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <TopoDS_Shape.hxx>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// ——— 辅助函数 ———

/// 等待条件为真或超时（轮询）
static bool waitUntil(std::function<bool()> cond,
                      std::chrono::milliseconds timeout = 3000ms)
{
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!cond()) {
        if (std::chrono::steady_clock::now() >= deadline) return false;
        std::this_thread::sleep_for(10ms);
    }
    return true;
}

// ——— T72-1 ———

/// 多脏对象 → 每对象独立提交任务 → ResultChannel 收到等量 post
TEST(BatchMeshTest, NdirtyIds_YieldNResultChannelPosts)
{
    app::Document doc;
    app::DependencyGraph graph;
    doc.setName("T72-1");

    // 在三个 ID 上标脏（无需真实文档对象，仅测试计数语义）
    const int N = 3;
    std::vector<foundation::UUID> ids;
    for (int i = 0; i < N; ++i) {
        ids.push_back(foundation::UUID::generate());
        graph.markDirty(ids.back());
    }

    task::WorkerGroup workers(2);
    task::ResultChannel channel;

    // 构建快照 + 提交 N 个独立任务（模拟 T72 asyncFn 模式）
    auto snap = std::make_shared<app::DocumentSnapshot>(
        app::makeDocumentSnapshot(doc, graph));
    graph.clearDirty();
    const auto version = snap->version;

    // 每个脏 ID 提交独立任务（即使快照中无对应 PipePoint，任务仍能提交）
    for (const auto& id : snap->dependencyGraph.dirtyIds) {
        (void)id;
        workers.submit([version, &channel](const task::CancellationToken& token) {
            if (token.isCancellationRequested()) return;
            // 模拟几何推导耗时
            std::this_thread::sleep_for(5ms);
            channel.post(version, []() { /* 哨兵 applyFn */ });
        });
    }

    // 等待所有任务提交结果
    bool done = waitUntil([&]() {
        return channel.pendingCount() >= static_cast<std::size_t>(N);
    });
    EXPECT_TRUE(done) << "未在超时前收到 " << N << " 个 ResultChannel post";

    EXPECT_EQ(channel.pendingCount(), static_cast<std::size_t>(N));

    workers.waitForIdle();
    workers.shutdown();
}

// ——— T72-2 ———

/// WorkerGroup(2) 可并发处理两个任务（批量不串行）
TEST(BatchMeshTest, WorkerGroup2_ConcurrentExecution)
{
    task::WorkerGroup workers(2);
    task::ResultChannel channel;

    std::atomic<int> startedCount{0};
    std::mutex startMutex;
    std::condition_variable startCv;

    // 两个任务各 sleep 80ms；串行需 ≥160ms，并行应 <100ms（留 50ms 余量）
    for (int i = 0; i < 2; ++i) {
        workers.submit([&startedCount, &startMutex, &startCv, &channel, i]
                       (const task::CancellationToken& token) {
            {
                std::unique_lock<std::mutex> lk(startMutex);
                ++startedCount;
                startCv.notify_all();
            }
            if (token.isCancellationRequested()) return;
            std::this_thread::sleep_for(80ms);
            channel.post(0, [i]() { (void)i; });
        });
    }

    auto t0 = std::chrono::steady_clock::now();
    // 等待至少 2 个任务均已启动（双线程并行关键验证点）
    {
        std::unique_lock<std::mutex> lk(startMutex);
        bool twoStarted = startCv.wait_for(lk, 500ms,
            [&]() { return startedCount.load() >= 2; });
        EXPECT_TRUE(twoStarted) << "两个任务未能同时启动，WorkerGroup 线程数可能<2";
    }

    workers.waitForIdle();
    auto elapsed = std::chrono::steady_clock::now() - t0;

    // 并行则总耗时 ≈ 80ms（远低于串行 160ms）
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    EXPECT_LT(elapsedMs, 150) << "总耗时 " << elapsedMs << "ms 超过并发预期，疑似串行";

    EXPECT_EQ(channel.pendingCount(), 2u);
    workers.shutdown();
}

// ——— T72-3 ———

/// ShapeMesher::mesh() 在后台线程中可正常调用并返回非空结果
TEST(BatchMeshTest, ShapeMesher_InBackground_ReturnsNonEmptyMesh)
{
    task::WorkerGroup workers(1);

    // 后台线程构建 OCCT 圆柱 → 调用 ShapeMesher
    std::atomic<bool> meshValid{false};
    std::atomic<std::size_t> vertexCount{0u};

    workers.submit([&meshValid, &vertexCount](const task::CancellationToken& token) {
        if (token.isCancellationRequested()) return;
        // 在后台线程创建简单几何（直径 50mm，高 200mm）
        BRepPrimAPI_MakeCylinder cylMaker(25.0, 200.0);
        TopoDS_Shape shape = cylMaker.Shape();
        ASSERT_FALSE(shape.IsNull());

        // 后台调用 ShapeMesher（T72 核心：不阻塞主线程）
        geometry::MeshData mesh = geometry::ShapeMesher::mesh(shape, 1.0);

        meshValid.store(!mesh.vertices.empty() && !mesh.indices.empty());
        vertexCount.store(mesh.vertices.size());
    });

    workers.waitForIdle();
    workers.shutdown();

    EXPECT_TRUE(meshValid.load()) << "后台线程中 ShapeMesher::mesh() 未能生成有效网格";
    EXPECT_GT(vertexCount.load(), 0u);
}

// ——— T72-4 ———

/// 批量并行任务全部完成后 drain 正确交付所有结果，不遗漏
TEST(BatchMeshTest, BatchParallelDrain_AllResultsDelivered)
{
    app::Document doc;
    app::DependencyGraph graph;
    doc.setName("T72-4");

    const int N = 4;
    task::WorkerGroup workers(2);
    task::ResultChannel channel;
    task::SceneUpdateAdapter adapter(channel,
        [&doc]() { return doc.currentVersion(); });

    const auto version = doc.currentVersion();
    std::atomic<int> applyCount{0};

    // 提交 N 个并行任务，每个向 channel 投递一个计数 applyFn
    for (int i = 0; i < N; ++i) {
        workers.submit([version, &channel, &applyCount]
                       (const task::CancellationToken& token) {
            if (token.isCancellationRequested()) return;
            std::this_thread::sleep_for(10ms);
            channel.post(version, [&applyCount]() { ++applyCount; });
        });
    }

    // 等待所有任务完成
    workers.waitForIdle();

    // 全部 N 个结果应已在 channel 中
    EXPECT_EQ(channel.pendingCount(), static_cast<std::size_t>(N));

    // Drain：版本匹配 → 执行全部 applyFn
    std::size_t applied = adapter.drain();
    EXPECT_EQ(applied, static_cast<std::size_t>(N));
    EXPECT_EQ(applyCount.load(), N);

    // Drain 后 channel 应为空
    EXPECT_EQ(channel.pendingCount(), 0u);

    workers.shutdown();
}
