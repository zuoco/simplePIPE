// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

module;

#include <cstdint>
#include "foundation/Types.h"

export module pipecad.runtime.task;

export namespace app {
struct DocumentSnapshot;
}

export namespace task {
class CancellationToken;
class TaskHandle;
class TaskQueue;
class WorkerGroup;
struct ResultItem;
class ResultChannel;
class SceneUpdateAdapter;
}

export namespace pipecad::runtime::task {
using TaskId = ::foundation::UUID;
using DocumentVersion = std::uint64_t;
using DocumentSnapshot = ::app::DocumentSnapshot;
using CancellationToken = ::task::CancellationToken;
using TaskHandle = ::task::TaskHandle;
using TaskQueue = ::task::TaskQueue;
using WorkerGroup = ::task::WorkerGroup;
using ResultItem = ::task::ResultItem;
using ResultChannel = ::task::ResultChannel;
using SceneUpdateAdapter = ::task::SceneUpdateAdapter;

// T69: 结果回投与版本控制边界已建立
// 后台任务完成后调用 ResultChannel::post(submittedVersion, applyFn)
// 主线程通过 ResultChannel::drainFresh(doc.currentVersion()) 消费并丢弃过期结果
struct TaskBoundary final {};
}