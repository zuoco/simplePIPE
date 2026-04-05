// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

export module pipecad.framework.application;

export namespace app {
class Application;
class ProjectSerializer;
class StepExporter;
}

export namespace pipecad::framework {
using Application = ::app::Application;
using ProjectSerializer = ::app::ProjectSerializer;
using StepExporter = ::app::StepExporter;
}