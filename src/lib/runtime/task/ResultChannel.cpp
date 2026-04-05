// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "task/ResultChannel.h"

namespace task {

void ResultChannel::post(app::DocumentVersion submittedVersion, std::function<void()> applyFn) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back({submittedVersion, std::move(applyFn)});
}

std::size_t ResultChannel::drainFresh(app::DocumentVersion currentVersion) {
    std::deque<ResultItem> items;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        items = std::move(queue_);
    }

    std::size_t applied = 0;
    for (auto& item : items) {
        if (item.submittedVersion == currentVersion && item.applyFn) {
            item.applyFn();
            ++applied;
        }
        // 版本不匹配：静默丢弃，不执行 applyFn
    }
    return applied;
}

std::size_t ResultChannel::drainAll() {
    std::deque<ResultItem> items;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        items = std::move(queue_);
    }

    std::size_t applied = 0;
    for (auto& item : items) {
        if (item.applyFn) {
            item.applyFn();
            ++applied;
        }
    }
    return applied;
}

void ResultChannel::discard() {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
}

std::size_t ResultChannel::pendingCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

} // namespace task
