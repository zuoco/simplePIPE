// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "task/SceneUpdateAdapter.h"

namespace task {

SceneUpdateAdapter::SceneUpdateAdapter(ResultChannel& channel,
                                       VersionProvider versionProvider)
    : channel_(channel)
    , versionProvider_(std::move(versionProvider))
{
}

std::size_t SceneUpdateAdapter::drain()
{
    // 主线程调用 versionProvider_() 获取当前文档版本；
    // 版本校验在 ResultChannel::drainFresh() 内部完成（加锁安全）。
    return channel_.drainFresh(versionProvider_());
}

std::size_t SceneUpdateAdapter::drainAll()
{
    return channel_.drainAll();
}

void SceneUpdateAdapter::discard()
{
    channel_.discard();
}

std::size_t SceneUpdateAdapter::pendingCount() const
{
    return channel_.pendingCount();
}

} // namespace task
