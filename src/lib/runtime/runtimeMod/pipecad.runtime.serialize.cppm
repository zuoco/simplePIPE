// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

export module pipecad.runtime.serialize;

export namespace command {
class Command;
class CommandRegistry;
}

export namespace pipecad::runtime {
using Command = ::command::Command;
using CommandRegistry = ::command::CommandRegistry;
}