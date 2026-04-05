# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PipeCAD (`qml-vsg-occt`) — parametric pipeline modeling and stress analysis software for offshore oil/gas platforms. Built with OCCT (geometry) + VSG (Vulkan rendering) + VTK (analysis visualization) + Qt6/QML (UI).

## Build & Test Commands

First-time setup (installs pixi, dependencies, runs initial CMake configure):
```bash
bash scripts/setup.sh
```

Build and test:
```bash
pixi run build-debug          # Configure + compile debug build
pixi run build-release        # Configure + compile release build
pixi run test                 # Build debug + run all tests via ctest
pixi run clean                # Remove build/ directory
```

When an AI agent compiles, runs, or tests the project, it must redirect output to temporary log files first, wait for the command to finish, and then read the log files to determine success or failure:
```bash
mkdir -p tmp_build_logs
pixi run build-debug > tmp_build_logs/build.log 2>&1
pixi run test > tmp_build_logs/test.log 2>&1
tail -50 tmp_build_logs/build.log
tail -50 tmp_build_logs/test.log
rm -rf tmp_build_logs
```

The same rule applies to filtered test runs and application launches: do not inspect long-running stdout directly; read the redirected log file after the command exits.

Run a single test:
```bash
bash scripts/build.sh test -R <TestName>     # e.g. test -R Engine
# Or manually:
pixi shell && cd build/debug
ctest -R <TestName> --output-on-failure      # Filter by name pattern
./tests/test_<name>                          # Run directly for gdb debugging
```

Run the application:
```bash
./build/debug/src/apps/pipecad/pipecad
# or via build script:
bash scripts/build.sh run
```

The `scripts/build.sh` wrapper provides subcommands: `debug`, `release`, `test [-R <Filter>]`, `run`, `full` (clean+build+test), `clean`, `configure`, `status`. Verify environment with `bash scripts/setup.sh --verify`.

Build concurrency defaults to 6 parallel build jobs and 1 test job. Override with environment variables:
```bash
PIPECAD_BUILD_JOBS=4 pixi run build-debug
PIPECAD_TEST_JOBS=2 pixi run test
```

## Architecture

The codebase is mid-migration from a flat 8-layer structure to a `lib/` + `apps/` topology (Phase 4, T50–T77). Both coexist — old directories retain only a `CMakeLists.txt` creating ALIAS targets pointing to the new locations.

### Current directory structure (post-Phase 4 migration)

```
src/
  lib/                         # Shared libraries
    base/                      # Header-only: UUID, Variant, Math, Signal, Log
                               #   + C++20 module interfaces (lib_base_modules)
    platform/
      occt/                    # OCCT geometry wrappers + C++20 facade module
      vsg/                     # VSG Vulkan rendering + C++20 facade module
      vtk/                     # VTK analysis visualization + C++20 facade module
    runtime/                   # Document, DependencyGraph, Command system + C++20 modules
    framework/                 # Application singleton, Workbench system, serialization
  apps/
    pipecad/
      model/                   # Domain objects: PipePoint, Segment, Route, PipeSpec, Load hierarchy
      engine/                  # Pipeline builders (Bend, Tee, Valve, Beam, Reducer), TopologyManager,
                               #   ConstraintSolver, RecomputeEngine, ComponentCatalog
      workbench/               # Workbench definitions: DesignWorkbench, AnalysisWorkbench, etc.
      ui/                      # QML bridge: VsgQuickItem, VtkViewport, AppController, table/tree models
      main.cpp                 # Application entry point
```

Old directories (`src/foundation/`, `src/geometry/`, `src/model/`, `src/engine/`, `src/visualization/`, `src/vtk-visualization/`, `src/app/`, `src/command/`, `src/ui/`) still exist but only contain a `CMakeLists.txt` with `add_library(<old_name> ALIAS <new_target>)`. **Do not add source files to these old directories.**

### Dependency chain (new targets)

```
lib_base (INTERFACE)
  ├─ lib_platform_occt (STATIC)
  │    ├─ pipecad_app_model (STATIC)
  │    │    └─ pipecad_app_engine (STATIC)
  │    └─ lib_platform_vtk (STATIC)
  └─ lib_platform_vsg (STATIC, depends on lib_platform_vtk)
lib_runtime (STATIC, depends on pipecad_app_engine + lib_platform_vsg)
lib_framework (STATIC, depends on lib_runtime + lib_platform_vsg)
pipecad_app (STATIC, aggregates all above + pipecad_app_ui)
pipecad (executable)
```

### Key Domain Concepts

- **PipePoint**: Core typed document object with coordinates. All pipeline geometry derives from PipePoint sequences and PipeSpec properties.
- **Application singleton**: `app::Application::init()` creates the central singleton holding Document, DependencyGraph, TransactionManager, SelectionManager, WorkbenchManager.
- **Command system**: CommandStack + CommandRegistry + typed commands (PropertyCommands, StructuralCommands, InsertComponent). TransactionManager was cleaned up in Phase 3 (T10).
- **Recompute flow**: Command execution → RecomputeEngine → SceneManager callback. Changing a PipePoint triggers recompute of dependent geometry and VSG scene node updates.
- **Workbench system**: CadWorkbench, DesignWorkbench, SpecWorkbench, AnalysisWorkbench — switchable via WorkbenchManager. Each workbench defines its own toolbar actions, tree structure, and panels.
- **Dual rendering**: VSG (Vulkan) for 3D design view via `VsgQuickItem` + VTK for stress analysis via `VtkViewport`. Both are QML-exposed types registered as `PipeCAD` module.
- **Component templates**: `src/apps/pipecad/engine/templates/` contains per-component headers (Elbow, GateValve, Pipe, Reducer, etc.) used by ComponentCatalog.
- **C++20 modules**: Being introduced alongside traditional headers. Module interfaces exist for `base`, `platform/occt`, `platform/vsg`, `platform/vtk`, `runtime`, and `framework`.

### Test Organization

Tests in `tests/` follow a per-task naming pattern (`test_<name>.cpp` linked to specific library layer(s) via `tests/CMakeLists.txt`). Tests link to old ALIAS target names which resolve to new targets. Current test baseline: **42 tests, all passing**. Tests using Qt depend on `Qt6::Test`; all others use `GTest::Main`.

### UI Structure

QML files in `ui/`: `main.qml` entry, `components/` (reusable widgets), `panels/` (workbench-specific panels), `dialogs/`, `style/Theme.qml`. Panels switch based on active workbench — e.g., AnalysisTree/LoadTable/LoadCaseTable are visible in the Analysis workbench.

## C++ Coding Conventions

- C++17 (std::optional, std::variant, structured bindings), with C++20 modules being introduced
- Class names PascalCase, functions camelCase, member variables prefixed `m_`, private members may also use `_` suffix
- Constants: `kPascalCase` or `UPPER_SNAKE_CASE`
- File headers: `// Copyright 2024-2026 PipeCAD Contributors` + `// SPDX-License-Identifier: Apache-2.0`
- OCCT objects managed via `Handle<T>` (never raw pointers to Transient objects)
- VSG objects managed via `vsg::ref_ptr<T>`
- Signal/slot: use `foundation::Signal<T>` (lightweight, no Qt dependency)
- Public APIs require Doxygen comments
- OCCT exceptions caught via `Standard_Failure`, not `std::exception`
- OCCT is NOT thread-safe — multi-threaded access requires synchronization
- Geometry algorithms must handle numerical precision using OCCT constants (`Precision::Confusion()`)

## Task Workflow

The project uses a task-driven workflow tracked in `docs/tasks/`. When asked to "完成任务 TXX":
1. Read `docs/tasks/current.md` for handoff context
2. Check `docs/tasks/status.md` status table (before "完成记录索引" section) for task status
3. Read task details from `docs/tasks/phase4-lib-app-refactor/` (Phase 4), `docs/archive/task-specs/development-plan.md` (Phase 1-2), or `docs/archive/task-specs/command-pattern-design.md` (Phase 3)
4. Implement, then verify by redirecting build/test output into temporary files, waiting for the commands to finish, and reading the log files to determine whether they succeeded
5. Update status.md, archived log file (`docs/archive/task-logs/`), current.md, and commit with `feat: TXX — description`

Recommended verification pattern:
```bash
mkdir -p tmp_build_logs
pixi run build-debug > tmp_build_logs/build.log 2>&1
pixi run test > tmp_build_logs/test.log 2>&1
tail -50 tmp_build_logs/build.log
tail -50 tmp_build_logs/test.log
rm -rf tmp_build_logs
```

Commit message format: `feat: TXX — 中文描述` / `fix: TXX — 中文描述` / `docs: 中文描述`

## Dependencies

| Library | Version | Source |
|---------|---------|--------|
| OCCT | 8.0.0 | Pre-built in `lib/occt/` |
| VSG | 1.1.13 | Pre-built in `lib/vsg/` |
| VTK | 9.6.0 | pixi (conda-forge) |
| Qt6 | ≥6.5 | pixi (conda-forge) |
| GTest | * | pixi (conda-forge) |
| nlohmann_json | * | pixi (conda-forge) |

Build toolchain: pixi + CMake >=3.24 + Ninja, Clang, C++17.

## Reference Documents

- `docs/architecture.md` — Full architecture design (data model, layering, UI design)
- `docs/tasks/status.md` — Task status tracking with completion history
- `docs/tasks/current.md` — Current task handoff (AI-maintained, read this first)
- `docs/archive/task-specs/development-plan.md` — Phase 1-2 task specs
- `docs/archive/task-specs/command-pattern-design.md` — Phase 3 command pattern specification
- `docs/tasks/phase4-lib-app-refactor/` — Phase 4 task cards
- `lib/occt/AGENTS.md`, `lib/vsg/AGENTS.md`, `lib/vtk/AGENTS.md` — Library-specific API guides
