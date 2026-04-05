// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Document.h"
#include "task/ResultChannel.h"

#include <cstddef>
#include <functional>

namespace task {

/// 主线程场景更新适配器：将 ResultChannel 中的后台计算结果
/// 按文档版本校验后在主线程执行场景更新。
///
/// ## 线程所有权
///
/// **所有成员函数仅允许在主线程（Qt 事件线程）调用。**
/// ResultChannel::post() 由后台线程调用；本适配器负责主线程侧的消费。
/// 两者通过 ResultChannel 内部的 std::mutex 实现线程安全握手。
///
/// ## 同步边界（T70 规范）
///
/// | 操作                      | 调用线程  | 说明                                      |
/// |---------------------------|-----------|-------------------------------------------|
/// | ResultChannel::post()     | 后台线程  | 每次后台任务完成后调用                      |
/// | SceneUpdateAdapter::drain() | 主线程  | Qt 事件循环或定时器触发；消费新鲜结果        |
/// | versionProvider_()        | 主线程    | Document::currentVersion() 非线程安全       |
///
/// ## 典型使用示例
///
/// ```cpp
/// // 主线程初始化阶段
/// task::ResultChannel channel;
/// task::SceneUpdateAdapter adapter(channel,
///     [&doc]() { return doc.currentVersion(); });
///
/// // 主线程事件循环（Qt 槽或 QueuedConnection 触发点）
/// std::size_t applied = adapter.drain();
/// // applied == 0 表示无新鲜结果或全部已过期并丢弃
/// ```
///
/// ## 失效丢弃（Stale Result Discard）
///
/// drain() 将 submittedVersion（任务提交时捕获的版本号）与
/// currentVersion（drain 时文档当前版本）进行比对：
///
///   submittedVersion == currentVersion → 执行 applyFn（更新场景）
///   submittedVersion != currentVersion → 静默丢弃，不写入任何状态
///
/// 这确保后台任务运行期间发生的文档变更（命令执行、用户编辑）不会
/// 导致过期几何写入场景。
class SceneUpdateAdapter {
public:
    /// 文档版本提供者函数类型：由主线程调用，返回当前文档版本号。
    ///
    /// 通常绑定为 `[&doc]() { return doc.currentVersion(); }`。
    /// **此函数仅在主线程调用，不需要线程安全保护。**
    using VersionProvider = std::function<app::DocumentVersion()>;

    /// 构造适配器。
    ///
    /// @param channel          后台→主线程结果回投通道（生命周期须长于本对象）
    /// @param versionProvider  版本查询函数，在每次 drain() 时调用一次（主线程）
    SceneUpdateAdapter(ResultChannel& channel, VersionProvider versionProvider);

    // 禁止复制与移动（持有非拥有引用，语义上绑定到特定通道）
    SceneUpdateAdapter(const SceneUpdateAdapter&) = delete;
    SceneUpdateAdapter& operator=(const SceneUpdateAdapter&) = delete;

    /// 主线程调用：消费所有待处理结果，版本匹配则执行 applyFn，版本过期则静默丢弃。
    ///
    /// 典型调用频率：每帧或每次命令执行后，在主线程事件循环中调用一次。
    ///
    /// @return 实际执行的 applyFn 次数（已过期的结果不计入）
    std::size_t drain();

    /// 主线程调用：消费所有待处理结果，**不校验版本**（强制刷新场景）。
    ///
    /// 适用场景：文档重新打开、工作台切换、强制全量刷新。
    ///
    /// @return 实际执行的 applyFn 次数
    std::size_t drainAll();

    /// 主线程调用：丢弃所有待处理结果，不执行任何 applyFn。
    ///
    /// 适用场景：文档关闭、工作台销毁、测试清理。
    void discard();

    /// 当前通道中待处理的结果数量。
    ///
    /// 此方法线程安全（委托给 ResultChannel::pendingCount()），
    /// 但通常只在主线程调用（用于状态监控或断言）。
    std::size_t pendingCount() const;

private:
    ResultChannel&  channel_;
    VersionProvider versionProvider_;
};

} // namespace task
