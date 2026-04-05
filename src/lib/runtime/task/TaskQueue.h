// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "foundation/Types.h"

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

namespace task {

class CancellationToken;
class TaskHandle;

class TaskQueue {
public:
    using TaskId = foundation::UUID;
    using TaskFunction = std::function<void(const CancellationToken&)>;

    TaskQueue() = default;
    ~TaskQueue() = default;

    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;

    TaskHandle enqueue(TaskFunction fn, std::string name = {});
    void close();
    void cancelPending();

    bool isClosed() const;
    std::size_t pendingCount() const;

private:
    struct TaskState {
        TaskId id = foundation::UUID::generate();
        std::string name;
        TaskFunction fn;
        mutable std::mutex mutex;
        mutable std::condition_variable cv;
        bool started = false;
        bool finished = false;
        bool succeeded = false;
        bool cancelRequested = false;
        std::exception_ptr error;

        void requestCancel();
        bool isCancellationRequested() const;
        void wait() const;
        void markStarted();
        void markFinished(bool success, std::exception_ptr errorPtr = nullptr);
    };

    friend class CancellationToken;
    friend class TaskHandle;
    friend class WorkerGroup;

    /// 取出下一个任务，阻塞直到有任务可取、队列关闭或 stop_token 被请求。
    /// 返回空 shared_ptr 表示工作线程应退出。
    std::shared_ptr<TaskState> take(std::stop_token st = {});

    mutable std::mutex mutex_;
    std::condition_variable_any cv_;
    std::deque<std::shared_ptr<TaskState>> pending_;
    bool closed_ = false;
};

class CancellationToken {
public:
    CancellationToken() = default;
    explicit CancellationToken(std::weak_ptr<TaskQueue::TaskState> state)
        : state_(std::move(state)) {}

    bool isCancellationRequested() const;

private:
    std::weak_ptr<TaskQueue::TaskState> state_;
};

class TaskHandle {
public:
    TaskHandle() = default;
    explicit TaskHandle(std::shared_ptr<TaskQueue::TaskState> state)
        : state_(std::move(state)) {}

    const foundation::UUID& id() const;
    std::string name() const;
    void cancel() const;
    void wait() const;
    bool valid() const { return static_cast<bool>(state_); }
    bool isStarted() const;
    bool isFinished() const;
    bool isCancellationRequested() const;
    bool succeeded() const;

private:
    std::shared_ptr<TaskQueue::TaskState> state_;
};

class WorkerGroup {
public:
    explicit WorkerGroup(std::size_t threadCount);
    ~WorkerGroup();

    WorkerGroup(const WorkerGroup&) = delete;
    WorkerGroup& operator=(const WorkerGroup&) = delete;

    TaskHandle submit(TaskQueue::TaskFunction fn, std::string name = {});
    void waitForIdle();
    void shutdown(bool cancelPending = true);

    bool isRunning() const;
    std::size_t threadCount() const { return threadCount_; }

private:
    void workerLoop(std::stop_token st);
    void registerActiveTask(const std::shared_ptr<TaskQueue::TaskState>& state);
    void unregisterActiveTask(const foundation::UUID& id);
    void cancelActiveTasks();

    const std::size_t threadCount_;
    TaskQueue queue_;
    mutable std::mutex stateMutex_;
    std::condition_variable idleCv_;
    std::vector<std::jthread> workers_;
    std::vector<std::shared_ptr<TaskQueue::TaskState>> activeTasks_;
    std::size_t activeCount_ = 0;
    bool running_ = true;
};

} // namespace task