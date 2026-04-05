// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Document.h"

#include <cstddef>
#include <deque>
#include <functional>
#include <mutex>

namespace task {

/// 后台任务回投到主线程的结果条目。
///
/// submittedVersion 为任务提交时捕获的文档版本号；
/// applyFn 为主线程接收结果后执行的回投函数（不跨线程）。
struct ResultItem {
    app::DocumentVersion submittedVersion = 0;
    std::function<void()> applyFn;
};

/// 线程安全的结果回投通道（后台→主线程单向队列）。
///
/// 典型用法：
///   1. 主线程提交任务时记录 submittedVersion = doc.currentVersion()
///   2. 后台任务完成后调用 channel.post(submittedVersion, applyFn)
///   3. 主线程在事件循环中调用 channel.drainFresh(doc.currentVersion())
///      — 版本匹配则执行 applyFn，版本过期则静默丢弃
///
/// 线程安全：post() 和 drain*()/discard() 可安全并发调用。
class ResultChannel {
public:
    ResultChannel() = default;

    ResultChannel(const ResultChannel&) = delete;
    ResultChannel& operator=(const ResultChannel&) = delete;

    /// 后台线程调用：将 applyFn 连同提交时的文档版本号一起放入队列。
    void post(app::DocumentVersion submittedVersion, std::function<void()> applyFn);

    /// 主线程调用：取出所有结果，submittedVersion == currentVersion 则执行，否则丢弃。
    /// 返回实际执行次数（已过期的结果不计入）。
    std::size_t drainFresh(app::DocumentVersion currentVersion);

    /// 主线程调用：取出并执行所有结果，不进行版本校验。适用于强制刷新场景。
    /// 返回执行次数。
    std::size_t drainAll();

    /// 主线程调用：丢弃所有待处理结果，不执行 applyFn。
    void discard();

    /// 当前队列中待处理的结果数量（线程安全）。
    std::size_t pendingCount() const;

private:
    mutable std::mutex mutex_;
    std::deque<ResultItem> queue_;
};

} // namespace task
