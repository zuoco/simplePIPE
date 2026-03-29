# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PipeCAD (`qml-vsg-occt`) — parametric pipeline modeling and stress analysis software for offshore oil/gas platforms. Built with OCCT (geometry) + VSG (Vulkan rendering) + VTK (analysis visualization) + Qt6/QML (UI).

## Build & Test Commands

```bash
pixi run build-debug          # Configure + compile debug build
pixi run build-release        # Configure + compile release build
pixi run test                 # Build debug + run all tests via ctest
pixi run clean                # Remove build/ directory
```

Run a single test:
```bash
pixi shell                    # Enter environment shell
cd build/debug
ctest -R <TestName> --output-on-failure   # Filter by name pattern
./tests/test_<name>                        # Run directly for gdb debugging
```

## Architecture

7-layer dependency chain (bottom to top), each compiled as a static library under `src/`:

```
foundation → geometry → model → engine → visualization → app → ui → pipecad_app
                                              vtk-visualization ↗
```

| Layer | Directory | Purpose |
|-------|-----------|---------|
| Foundation | `src/foundation/` | UUID, Variant, Math, Signal, Log base types |
| Geometry | `src/geometry/` | OCCT wrappers: ShapeBuilder, BooleanOps, StepIO, ShapeMesher |
| Model | `src/model/` | Header-only domain objects: PipePoint, Segment, Route, PipeSpec, Load hierarchy |
| Engine | `src/engine/` | Pipeline builders (Bend, Tee, Valve, Beam), ComponentCatalog, TopologyManager, ConstraintSolver, RecomputeEngine |
| Visualization | `src/visualization/` | VSG rendering: OcctToVsg, SceneManager, PickHandler, ViewManager |
| VTK Visualization | `src/vtk-visualization/` | VTK analysis viewport: BeamMeshBuilder, VtkSceneManager |
| Application | `src/app/` | Document, Workbench system (Cad/Design/Spec/Analysis), ProjectSerializer, SelectionManager |
| UI | `src/ui/` | QML bridge: VsgQuickItem, VtkViewport, table/tree models, AppController |

### Key Domain Concepts

- **PipePoint**: Core typed document object with coordinates. All pipeline geometry derives from PipePoint sequences and PipeSpec properties.
- **Workbench system**: CadWorkbench, DesignWorkbench, SpecWorkbench, AnalysisWorkbench — switchable via WorkbenchManager.
- **Dual rendering**: VSG (Vulkan) for 3D design view + VTK for stress analysis visualization.

### UI Structure

QML files in `ui/`: `main.qml` entry, `components/` (reusable widgets), `panels/` (feature panels), `dialogs/`, `style/Theme.qml`.

## C++ Coding Conventions

- C++17 (std::optional, std::variant, structured bindings)
- Class names PascalCase, functions camelCase, member variables prefixed `m_`
- OCCT objects managed via `Handle<T>` (never raw pointers to Transient objects)
- VSG objects managed via `osg::ref_ptr<T>` (note: this project uses VSG not OSG despite the type name)
- Public APIs require Doxygen comments
- OCCT exceptions caught via `Standard_Failure`, not `std::exception`
- OCCT is NOT thread-safe — multi-threaded access requires synchronization
- Geometry algorithms must handle numerical precision using OCCT constants (`Precision::Confusion()`)

## Task Workflow (AGENTS.md)

The project uses a task-driven workflow tracked in `docs/tasks/`. When asked to "完成任务 TXX":
1. Read `docs/tasks/current.md` for handoff context
2. Check `docs/tasks/status.md` (first 74 lines) for task status
3. Read task details from `docs/development-plan.md`
4. Implement, then verify with `pixi run build-debug && pixi run test`
5. Update status.md, log file, current.md, and commit with `feat: TXX — description`

Commit message format: `feat: TXX — 中文描述` / `fix: TXX — 中文描述` / `docs: 中文描述`

## Dependencies

| Library | Version | Source |
|---------|---------|--------|
| OCCT | 8.0.0 | Pre-built in `lib/occt/` |
| VSG | 1.1.13 | Pre-built in `lib/vsg/` |
| VTK | 9.6.0 | Pre-built in `lib/vtk/` |
| Qt6 | ≥6.5 | pixi (conda-forge) |
| GTest | * | pixi (conda-forge) |
| nlohmann_json | * | pixi (conda-forge) |

Build toolchain: pixi + CMake ≥3.24 + Ninja, C++17.
