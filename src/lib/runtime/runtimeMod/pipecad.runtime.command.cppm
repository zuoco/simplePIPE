// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

export module pipecad.runtime.command;

export namespace command {
class BatchSetPropertyCommand;
class Command;
struct CommandContext;
class CommandRegistry;
struct CommandResult;
class CommandStack;
enum class CommandType;
class CreatePipePointCommand;
class DeletePipePointCommand;
class InsertComponentCommand;
class MacroCommand;
class SetPropertyCommand;
}

export namespace pipecad::runtime {
using BatchSetPropertyCommand = ::command::BatchSetPropertyCommand;
using Command = ::command::Command;
using CommandContext = ::command::CommandContext;
using CommandRegistry = ::command::CommandRegistry;
using CommandResult = ::command::CommandResult;
using CommandStack = ::command::CommandStack;
using CommandType = ::command::CommandType;
using CreatePipePointCommand = ::command::CreatePipePointCommand;
using DeletePipePointCommand = ::command::DeletePipePointCommand;
using InsertComponentCommand = ::command::InsertComponentCommand;
using MacroCommand = ::command::MacroCommand;
using SetPropertyCommand = ::command::SetPropertyCommand;
}