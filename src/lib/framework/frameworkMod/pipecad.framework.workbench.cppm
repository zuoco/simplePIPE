// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

export module pipecad.framework.workbench;

export namespace app {
class AnalysisWorkbench;
class CadWorkbench;
class DesignWorkbench;
class SpecWorkbench;
struct ToolbarAction;
enum class ViewportType;
class Workbench;
class WorkbenchManager;
}

export namespace pipecad::framework {
using AnalysisWorkbench = ::app::AnalysisWorkbench;
using CadWorkbench = ::app::CadWorkbench;
using DesignWorkbench = ::app::DesignWorkbench;
using SpecWorkbench = ::app::SpecWorkbench;
using ToolbarAction = ::app::ToolbarAction;
using ViewportType = ::app::ViewportType;
using Workbench = ::app::Workbench;
using WorkbenchManager = ::app::WorkbenchManager;
}