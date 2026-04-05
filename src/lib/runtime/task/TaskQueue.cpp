// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "task/TaskQueue.h"

#include <algorithm>
#include <stdexcept>
#include <thread>

namespace task {

void TaskQueue::TaskState::requestCancel() {
    std::lock_guard<std::mutex> lock(mutex);
    cancelRequested = true;
}

bool TaskQueue::TaskState::isCancellationRequested() const {
    std::lock_guard<std::mutex> lock(mutex);
    return cancelRequested;
}

void TaskQueue::TaskState::wait() const {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this]() { return finished; });
}

void TaskQueue::TaskState::markStarted() {
    std::lock_guard<std::mutex> lock(mutex);
    started = true;
}

void TaskQueue::TaskState::markFinished(bool success, std::exception_ptr errorPtr) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        finished = true;
        succeeded = success;
        error = std::move(errorPtr);
    }
    cv.notify_all();
}

TaskHandle TaskQueue::enqueue(TaskFunction fn, std::string name) {
    if (!fn) {
        throw std::invalid_argument("TaskQueue::enqueue requires a valid task");
    }

    auto state = std::make_shared<TaskState>();
    state->name = std::move(name);
    state->fn = std::move(fn);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) {
            throw std::runtime_error("TaskQueue is closed");
        }
        pending_.push_back(state);
    }
    cv_.notify_one();
    return TaskHandle(state);
}

void TaskQueue::close() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
    }
    cv_.notify_all();
}

void TaskQueue::cancelPending() {
    std::deque<std::shared_ptr<TaskState>> cancelled;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cancelled.swap(pending_);
    }

    for (const auto& task : cancelled) {
        task->requestCancel();
        task->markFinished(false);
    }

    cv_.notify_all();
}

bool TaskQueue::isClosed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return closed_;
}

std::size_t TaskQueue::pendingCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_.size();
}

std::shared_ptr<TaskQueue::TaskState> TaskQueue::take(std::stop_token st) {
    std::unique_lock<std::mutex> lock(mutex_);
    // C++20 condition_variable_any::wait 的 stop_token 重载：
    // 当谓词为真，或 stop_requested() 变为 true 时均会返回。
    cv_.wait(lock, st, [this]() {
        return closed_ || !pending_.empty();
    });

    if (pending_.empty()) {
        return {};
    }

    auto next = pending_.front();
    pending_.pop_front();
    return next;
}

bool CancellationToken::isCancellationRequested() const {
    auto state = state_.lock();
    return state ? state->isCancellationRequested() : true;
}

const foundation::UUID& TaskHandle::id() const {
    if (!state_) {
        throw std::runtime_error("Invalid TaskHandle");
    }
    return state_->id;
}

std::string TaskHandle::name() const {
    return state_ ? state_->name : std::string{};
}

void TaskHandle::cancel() const {
    if (state_) {
        state_->requestCancel();
    }
}

void TaskHandle::wait() const {
    if (state_) {
        state_->wait();
    }
}

bool TaskHandle::isStarted() const {
    if (!state_) {
        return false;
    }
    std::lock_guard<std::mutex> lock(state_->mutex);
    return state_->started;
}

bool TaskHandle::isFinished() const {
    if (!state_) {
        return false;
    }
    std::lock_guard<std::mutex> lock(state_->mutex);
    return state_->finished;
}

bool TaskHandle::isCancellationRequested() const {
    return state_ ? state_->isCancellationRequested() : false;
}

bool TaskHandle::succeeded() const {
    if (!state_) {
        return false;
    }
    std::lock_guard<std::mutex> lock(state_->mutex);
    return state_->finished && state_->succeeded;
}

WorkerGroup::WorkerGroup(std::size_t threadCount)
    : threadCount_(threadCount == 0 ? 1 : threadCount) {
    workers_.reserve(threadCount_);
    for (std::size_t index = 0; index < threadCount_; ++index) {
        // std::jthread 构造时自动传入 stop_token 参数
        workers_.emplace_back([this](std::stop_token st) {
            workerLoop(st);
        });
    }
}

WorkerGroup::~WorkerGroup() {
    shutdown(true);
}

TaskHandle WorkerGroup::submit(TaskQueue::TaskFunction fn, std::string name) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!running_) {
        throw std::runtime_error("WorkerGroup has been shut down");
    }
    return queue_.enqueue(std::move(fn), std::move(name));
}

void WorkerGroup::waitForIdle() {
    std::unique_lock<std::mutex> lock(stateMutex_);
    idleCv_.wait(lock, [this]() {
        return activeCount_ == 0 && queue_.pendingCount() == 0;
    });
}

void WorkerGroup::shutdown(bool cancelPending) {
    std::vector<std::jthread> workers;
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        if (!running_) {
            return;
        }
        running_ = false;
        workers = std::move(workers_);
    }

    if (cancelPending) {
        cancelActiveTasks();
        queue_.cancelPending();
    }
    queue_.close();

    // std::jthread 析构时自动调用 request_stop() + join()
    // workers 离开作用域时工作线程将收到 stop 信号并安全退出
    workers.clear();

    idleCv_.notify_all();
}

bool WorkerGroup::isRunning() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return running_;
}

void WorkerGroup::workerLoop(std::stop_token st) {
    while (true) {
        auto task = queue_.take(st);
        if (!task) {
            return;
        }

        registerActiveTask(task);
        task->markStarted();

        if (task->isCancellationRequested()) {
            task->markFinished(false);
            unregisterActiveTask(task->id);
            continue;
        }

        try {
            task->fn(CancellationToken(task));
            const bool success = !task->isCancellationRequested();
            task->markFinished(success);
        } catch (...) {
            task->markFinished(false, std::current_exception());
        }

        unregisterActiveTask(task->id);
    }
}

void WorkerGroup::registerActiveTask(const std::shared_ptr<TaskQueue::TaskState>& state) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    ++activeCount_;
    activeTasks_.push_back(state);
}

void WorkerGroup::unregisterActiveTask(const foundation::UUID& id) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    activeTasks_.erase(
        std::remove_if(activeTasks_.begin(), activeTasks_.end(), [&](const auto& task) {
            return task && task->id == id;
        }),
        activeTasks_.end());
    if (activeCount_ > 0) {
        --activeCount_;
    }
    if (activeCount_ == 0 && queue_.pendingCount() == 0) {
        idleCv_.notify_all();
    }
}

void WorkerGroup::cancelActiveTasks() {
    std::lock_guard<std::mutex> lock(stateMutex_);
    for (const auto& task : activeTasks_) {
        if (task) {
            task->requestCancel();
        }
    }
}

} // namespace task