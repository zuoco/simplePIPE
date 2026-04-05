// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace command {

/// 命令类型枚举（仅用于 C++ virtual type() 返回值）
/// CommandRegistry 的注册 key 统一使用字符串，两套标识系统各有明确职责
enum class CommandType {
    SetProperty,        ///< 属性修改
    BatchSetProperty,   ///< 批量属性修改
    CreatePipePoint,    ///< 创建管点
    DeletePipePoint,    ///< 删除管点
    CreateRoute,        ///< 创建路由
    DeleteRoute,        ///< 删除路由
    CreateSegment,      ///< 创建段
    DeleteSegment,      ///< 删除段
    CreateLoad,         ///< 创建载荷
    DeleteLoad,         ///< 删除载荷
    CreateAccessory,    ///< 创建附属构件
    DeleteAccessory,    ///< 删除附属构件
    ModifyTopology,     ///< 拓扑变更
    Macro               ///< 宏命令（组合操作，含 InsertComponent）
};

} // namespace command
