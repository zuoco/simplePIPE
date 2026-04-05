// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "app/DependencyGraph.h"
#include "app/Document.h"
#include "app/DocumentSnapshot.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Route.h"
#include "model/Segment.h"
#include "task/TaskQueue.h"
#include "task/ResultChannel.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

namespace {

using namespace std::chrono_literals;

TEST(DocumentSnapshot, DocumentVersionTracksStructureAndPropertyChanges) {
    app::Document document;
    EXPECT_EQ(document.currentVersion(), 0u);

    auto pipeSpec = std::make_shared<model::PipeSpec>("CS150");
    document.addObject(pipeSpec);
    EXPECT_EQ(document.currentVersion(), 1u);

    auto point = std::make_shared<model::PipePoint>("P1");
    document.addObject(point);
    EXPECT_EQ(document.currentVersion(), 2u);

    point->setName("P1-Renamed");
    EXPECT_EQ(document.currentVersion(), 3u);

    document.setName("SnapshotDoc");
    EXPECT_EQ(document.currentVersion(), 4u);

    document.removeObject(point->id());
    EXPECT_EQ(document.currentVersion(), 5u);

    point->setName("Detached");
    EXPECT_EQ(document.currentVersion(), 5u);
}

TEST(DocumentSnapshot, FreezesPipeTopologyAndDependencyView) {
    app::Document document;
    app::DependencyGraph dependencyGraph;

    auto pipeSpec = std::make_shared<model::PipeSpec>("Spec-A");
    pipeSpec->setOd(168.3);
    pipeSpec->setWallThickness(7.11);
    pipeSpec->setMaterial("A106-B");
    document.addObject(pipeSpec);

    auto route = std::make_shared<model::Route>("Route-1");
    auto segment = std::make_shared<model::Segment>("Seg-1");
    auto p1 = std::make_shared<model::PipePoint>("P1", model::PipePointType::Run, gp_Pnt(0.0, 0.0, 0.0));
    auto p2 = std::make_shared<model::PipePoint>("P2", model::PipePointType::Bend, gp_Pnt(1000.0, 0.0, 0.0));
    p1->setPipeSpec(pipeSpec);
    p2->setPipeSpec(pipeSpec);
    p2->setParam("bendMultiplier", 1.5);
    segment->addPoint(p1);
    segment->addPoint(p2);
    route->addSegment(segment);

    document.addObject(route);
    document.addObject(segment);
    document.addObject(p1);
    document.addObject(p2);

    dependencyGraph.addDependency(p2->id(), p1->id());
    dependencyGraph.markDirty(p1->id());

    const auto snapshot = app::makeDocumentSnapshot(document, dependencyGraph);

    EXPECT_EQ(snapshot.version, document.currentVersion());
    ASSERT_EQ(snapshot.pipeSpecs.size(), 1u);
    ASSERT_EQ(snapshot.pipePoints.size(), 2u);
    ASSERT_EQ(snapshot.segments.size(), 1u);
    ASSERT_EQ(snapshot.routes.size(), 1u);

    const auto* pointSnapshot = snapshot.findPipePoint(p2->id());
    ASSERT_NE(pointSnapshot, nullptr);
    ASSERT_TRUE(pointSnapshot->pipeSpecId.has_value());
    EXPECT_EQ(pointSnapshot->pipeSpecId.value(), pipeSpec->id());
    EXPECT_EQ(foundation::variantToDouble(pointSnapshot->typeParams.at("bendMultiplier")), 1.5);

    const auto* segmentSnapshot = snapshot.findSegment(segment->id());
    ASSERT_NE(segmentSnapshot, nullptr);
    ASSERT_EQ(segmentSnapshot->pointIds.size(), 2u);
    EXPECT_EQ(segmentSnapshot->pointIds.front(), p1->id());
    EXPECT_EQ(segmentSnapshot->pointIds.back(), p2->id());

    const auto* routeSnapshot = snapshot.findRoute(route->id());
    ASSERT_NE(routeSnapshot, nullptr);
    ASSERT_EQ(routeSnapshot->segmentIds.size(), 1u);
    EXPECT_EQ(routeSnapshot->segmentIds.front(), segment->id());

    const auto* dependencyNode = snapshot.dependencyGraph.findNode(p2->id());
    ASSERT_NE(dependencyNode, nullptr);
    ASSERT_EQ(dependencyNode->dependsOn.size(), 1u);
    EXPECT_EQ(dependencyNode->dependsOn.front(), p1->id());
    ASSERT_EQ(snapshot.dependencyGraph.dirtyIds.size(), 2u);
    EXPECT_EQ(snapshot.dependencyGraph.dirtyIds.front(), p1->id());
    EXPECT_EQ(snapshot.dependencyGraph.dirtyIds.back(), p2->id());
}

TEST(TaskQueue, EnqueueTracksPendingState) {
    task::TaskQueue queue;
    auto handle = queue.enqueue([](const task::CancellationToken&) {}, "noop");

    EXPECT_TRUE(handle.valid());
    EXPECT_EQ(queue.pendingCount(), 1u);

    queue.cancelPending();
    handle.wait();
    EXPECT_TRUE(handle.isFinished());
    EXPECT_FALSE(handle.succeeded());
    EXPECT_EQ(queue.pendingCount(), 0u);

    queue.close();
    EXPECT_TRUE(queue.isClosed());
}

TEST(WorkerGroup, SubmittedTaskRunsAndCanBeWaited) {
    task::WorkerGroup workers(1);
    std::atomic<int> counter{0};

    auto handle = workers.submit([&counter](const task::CancellationToken& token) {
        if (!token.isCancellationRequested()) {
            counter.fetch_add(1);
        }
    }, "increment");

    handle.wait();
    EXPECT_TRUE(handle.isFinished());
    EXPECT_TRUE(handle.succeeded());
    EXPECT_EQ(counter.load(), 1);

    workers.waitForIdle();
    workers.shutdown();
}

TEST(WorkerGroup, PendingTaskCanBeCancelledBeforeExecution) {
    task::WorkerGroup workers(1);
    std::atomic<bool> releaseFirst{false};
    std::atomic<int> executed{0};

    auto first = workers.submit([&](const task::CancellationToken& token) {
        while (!releaseFirst.load() && !token.isCancellationRequested()) {
            std::this_thread::sleep_for(1ms);
        }
        if (!token.isCancellationRequested()) {
            executed.fetch_add(1);
        }
    }, "blocking");

    auto second = workers.submit([&](const task::CancellationToken& token) {
        if (!token.isCancellationRequested()) {
            executed.fetch_add(1);
        }
    }, "cancelled");

    second.cancel();
    releaseFirst.store(true);

    first.wait();
    second.wait();

    EXPECT_TRUE(first.succeeded());
    EXPECT_FALSE(second.succeeded());
    EXPECT_EQ(executed.load(), 1);

    workers.shutdown();
}

TEST(WorkerGroup, ShutdownCancelsPendingAndRequestsRunningTaskExit) {
    task::WorkerGroup workers(1);
    std::atomic<bool> observedCancel{false};

    auto running = workers.submit([&](const task::CancellationToken& token) {
        while (!token.isCancellationRequested()) {
            std::this_thread::sleep_for(1ms);
        }
        observedCancel.store(true);
    }, "running");

    while (!running.isStarted()) {
        std::this_thread::sleep_for(1ms);
    }

    auto pending = workers.submit([](const task::CancellationToken&) {
        FAIL() << "pending task should be cancelled during shutdown";
    }, "pending");

    workers.shutdown(true);

    running.wait();
    pending.wait();

    EXPECT_TRUE(observedCancel.load());
    EXPECT_FALSE(pending.succeeded());
    EXPECT_FALSE(workers.isRunning());
}

// ─── T69: ResultChannel 结果回投与任务版本控制 ────────────────────────────────

TEST(ResultChannel, DrainFreshAppliesMatchingVersionOnly) {
    task::ResultChannel channel;

    int applied = 0;
    channel.post(5u, [&]() { ++applied; }); // 版本匹配
    channel.post(5u, [&]() { ++applied; }); // 版本匹配
    channel.post(3u, [&]() { ++applied; }); // 版本过期

    const std::size_t count = channel.drainFresh(5u);

    EXPECT_EQ(count, 2u);
    EXPECT_EQ(applied, 2);
    EXPECT_EQ(channel.pendingCount(), 0u);
}

TEST(ResultChannel, DrainFreshDiscardsAllWhenVersionChanged) {
    task::ResultChannel channel;

    int applied = 0;
    channel.post(3u, [&]() { ++applied; });
    channel.post(3u, [&]() { ++applied; });

    const std::size_t count = channel.drainFresh(7u); // 文档已更新

    EXPECT_EQ(count, 0u);
    EXPECT_EQ(applied, 0);
    EXPECT_EQ(channel.pendingCount(), 0u);
}

TEST(ResultChannel, DrainAllIgnoresVersion) {
    task::ResultChannel channel;

    int applied = 0;
    channel.post(1u, [&]() { ++applied; });
    channel.post(4u, [&]() { ++applied; });
    channel.post(9u, [&]() { ++applied; });

    const std::size_t count = channel.drainAll();

    EXPECT_EQ(count, 3u);
    EXPECT_EQ(applied, 3);
    EXPECT_EQ(channel.pendingCount(), 0u);
}

TEST(ResultChannel, DiscardEmptiesQueueWithoutExecuting) {
    task::ResultChannel channel;

    int applied = 0;
    channel.post(2u, [&]() { ++applied; });
    channel.post(2u, [&]() { ++applied; });

    EXPECT_EQ(channel.pendingCount(), 2u);
    channel.discard();
    EXPECT_EQ(channel.pendingCount(), 0u);
    EXPECT_EQ(applied, 0);
}

TEST(ResultChannel, ThreadSafeMultipleProducersSingleConsumer) {
    task::ResultChannel channel;
    std::atomic<int> posted{0};
    constexpr int kItems = 50;

    // 多线程并发 post
    std::vector<std::thread> producers;
    producers.reserve(5);
    for (int i = 0; i < 5; ++i) {
        producers.emplace_back([&]() {
            for (int j = 0; j < kItems / 5; ++j) {
                channel.post(1u, [&posted]() { posted.fetch_add(1); });
            }
        });
    }
    for (auto& t : producers) {
        t.join();
    }

    EXPECT_EQ(channel.pendingCount(), static_cast<std::size_t>(kItems));

    const std::size_t count = channel.drainFresh(1u);
    EXPECT_EQ(count, static_cast<std::size_t>(kItems));
    EXPECT_EQ(posted.load(), kItems);
}

TEST(ResultChannel, WorkerGroupPostsToChannelAndMainThreadDrains) {
    task::WorkerGroup workers(2);
    task::ResultChannel channel;

    // 任务提交时记录文档版本
    constexpr app::DocumentVersion kVersion = 10u;
    std::atomic<int> computed{0};
    int applied = 0;

    for (int i = 0; i < 4; ++i) {
        workers.submit([&channel, kVersion, &computed](const task::CancellationToken& token) {
            if (!token.isCancellationRequested()) {
                computed.fetch_add(1);
                channel.post(kVersion, [&channel]() {
                    (void)channel; // placeholder: 实际场景会更新场景节点
                });
            }
        }, "compute");
    }

    workers.waitForIdle();
    workers.shutdown(false);

    // 主线程以当前文档版本 drain —— 版本匹配，全部应用
    const std::size_t count = channel.drainFresh(kVersion);
    EXPECT_EQ(computed.load(), 4);
    EXPECT_EQ(count, 4u);
}

TEST(ResultChannel, StaleResultsAreDiscardedAfterDocumentUpdate) {
    task::WorkerGroup workers(1);
    task::ResultChannel channel;

    constexpr app::DocumentVersion kOldVersion = 3u;
    constexpr app::DocumentVersion kNewVersion = 7u; // 文档已更新

    int applied = 0;
    workers.submit([&channel, kOldVersion, &applied](const task::CancellationToken& token) {
        if (!token.isCancellationRequested()) {
            // 后台计算完成，回投旧版本的结果
            channel.post(kOldVersion, [&applied]() { ++applied; });
        }
    }, "stale-compute");

    workers.waitForIdle();
    workers.shutdown(false);

    // 主线程以新版本 drain，旧结果应被丢弃
    const std::size_t count = channel.drainFresh(kNewVersion);
    EXPECT_EQ(count, 0u);
    EXPECT_EQ(applied, 0);
}

} // namespace