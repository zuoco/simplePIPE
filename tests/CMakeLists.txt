find_package(GTest REQUIRED)

# 构建系统占位测试 — T01 验证
add_executable(test_build_system test_build_system.cpp)
target_link_libraries(test_build_system
    PRIVATE
        foundation
        GTest::GTest
        GTest::Main
)
target_compile_features(test_build_system PRIVATE cxx_std_17)
add_test(NAME BuildSystem COMMAND test_build_system)

# Foundation 层测试 — T02
add_executable(test_foundation test_foundation.cpp)
target_link_libraries(test_foundation
    PRIVATE
        foundation
        GTest::GTest
        GTest::Main
)
target_compile_features(test_foundation PRIVATE cxx_std_17)
add_test(NAME Foundation COMMAND test_foundation)

# Geometry 层测试 — T03
add_executable(test_geometry test_geometry.cpp)
target_link_libraries(test_geometry
    PRIVATE
        geometry
        GTest::GTest
        GTest::Main
)
target_include_directories(test_geometry PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_geometry PRIVATE cxx_std_17)
add_test(NAME Geometry COMMAND test_geometry)

# Mesh + STEP I/O 测试 — T04
add_executable(test_mesh_step test_mesh_step.cpp)
target_link_libraries(test_mesh_step
    PRIVATE
        geometry
        GTest::GTest
        GTest::Main
)
target_include_directories(test_mesh_step PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_mesh_step PRIVATE cxx_std_17)
add_test(NAME MeshStep COMMAND test_mesh_step)

# Model 层测试 — T05
add_executable(test_model test_model.cpp)
target_link_libraries(test_model
    PRIVATE
        model
        GTest::GTest
        GTest::Main
)
target_include_directories(test_model PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_model PRIVATE cxx_std_17)
add_test(NAME Model COMMAND test_model)

# Engine 层测试 — T07
add_executable(test_engine test_engine.cpp)
target_link_libraries(test_engine
    PRIVATE
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_engine PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_engine PRIVATE cxx_std_17)
add_test(NAME Engine COMMAND test_engine)

# 管件几何测试 — T08
add_executable(test_pipe_geometry test_pipe_geometry.cpp)
target_link_libraries(test_pipe_geometry
    PRIVATE
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_pipe_geometry PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_pipe_geometry PRIVATE cxx_std_17)
add_test(NAME PipeGeometry COMMAND test_pipe_geometry)

# 阀门/柔性接头/型钢梁/附件测试 — T09
add_executable(test_valve_flex_beam test_valve_flex_beam.cpp)
target_link_libraries(test_valve_flex_beam
    PRIVATE
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_valve_flex_beam PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_valve_flex_beam PRIVATE cxx_std_17)
add_test(NAME ValveFlexBeam COMMAND test_valve_flex_beam)

# ComponentCatalog 参数化构件模板库测试 — T31
add_executable(test_component_catalog test_component_catalog.cpp)
target_link_libraries(test_component_catalog
    PRIVATE
        engine
        geometry
        GTest::GTest
        GTest::Main
)
target_include_directories(test_component_catalog PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_component_catalog PRIVATE cxx_std_17)
add_test(NAME ComponentCatalog COMMAND test_component_catalog)

# 附属对象与梁测试 — T06
add_executable(test_accessory_beam test_accessory_beam.cpp)
target_link_libraries(test_accessory_beam
    PRIVATE
        model
        GTest::GTest
        GTest::Main
)
target_include_directories(test_accessory_beam PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_accessory_beam PRIVATE cxx_std_17)
add_test(NAME AccessoryBeam COMMAND test_accessory_beam)

# 拓扑管理与约束测试 — T10
add_executable(test_topology test_topology.cpp)
target_link_libraries(test_topology
    PRIVATE
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_topology PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_topology PRIVATE cxx_std_17)
add_test(NAME Topology COMMAND test_topology)

# OCCT→VSG 网格转换测试 — T11
add_executable(test_visualization test_visualization.cpp)
target_link_libraries(test_visualization
    PRIVATE
        visualization
        GTest::GTest
        GTest::Main
)
target_include_directories(test_visualization PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_visualization PRIVATE cxx_std_17)
add_test(NAME Visualization COMMAND test_visualization)

# VSG 场景管理测试 — T12
add_executable(test_scene_manager test_scene_manager.cpp)
target_link_libraries(test_scene_manager
    PRIVATE
        visualization
        GTest::GTest
        GTest::Main
)
target_compile_features(test_scene_manager PRIVATE cxx_std_17)
add_test(NAME SceneManager COMMAND test_scene_manager)

# 相机控制与场景基础设施测试 — T13
add_executable(test_camera_furniture test_camera_furniture.cpp)
target_link_libraries(test_camera_furniture
    PRIVATE
        visualization
        GTest::GTest
        GTest::Main
)
target_compile_features(test_camera_furniture PRIVATE cxx_std_17)
add_test(NAME CameraFurniture COMMAND test_camera_furniture)

# 3D 拾取与高亮测试 — T14
add_executable(test_pick_highlight test_pick_highlight.cpp)
target_link_libraries(test_pick_highlight
    PRIVATE
        visualization
        GTest::GTest
        GTest::Main
)
target_compile_features(test_pick_highlight PRIVATE cxx_std_17)
add_test(NAME PickHighlight COMMAND test_pick_highlight)

# VSG-QML 桥接测试 — T15
add_executable(test_vsg_qml_bridge test_vsg_qml_bridge.cpp)
target_link_libraries(test_vsg_qml_bridge
    PRIVATE
        ui
        Qt6::Test
        GTest::GTest
)
target_compile_features(test_vsg_qml_bridge PRIVATE cxx_std_17)
add_test(NAME VsgQmlBridge COMMAND test_vsg_qml_bridge)

# 应用层核心测试 — T16
add_executable(test_app_core test_app_core.cpp)
target_link_libraries(test_app_core
    PRIVATE
        app
        command
        GTest::GTest
        GTest::Main
)
target_include_directories(test_app_core PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_app_core PRIVATE cxx_std_17)
add_test(NAME AppCore COMMAND test_app_core)

# JSON 工程序列化测试 — T20
add_executable(test_project_serializer test_project_serializer.cpp)
target_link_libraries(test_project_serializer
    PRIVATE
        app
        GTest::GTest
        GTest::Main
)
target_include_directories(test_project_serializer PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_project_serializer PRIVATE cxx_std_17)
add_test(NAME ProjectSerializer COMMAND test_project_serializer)

# Load/LoadCase/LoadCombination 序列化测试 — T43
add_executable(test_load_serialization test_load_serialization.cpp)
target_link_libraries(test_load_serialization
    PRIVATE
        app
        GTest::GTest
        GTest::Main
)
target_include_directories(test_load_serialization PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_load_serialization PRIVATE cxx_std_17)
add_test(NAME LoadSerialization COMMAND test_load_serialization)

# STEP 导出测试 — T21
add_executable(test_step_exporter test_step_exporter.cpp)
target_link_libraries(test_step_exporter
    PRIVATE
        app
        GTest::GTest
        GTest::Main
)
target_include_directories(test_step_exporter PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_step_exporter PRIVATE cxx_std_17)
add_test(NAME StepExporter COMMAND test_step_exporter)

# 工作台系统 + QML 桥接测试 — T17
add_executable(test_workbench_bridge test_workbench_bridge.cpp)
target_link_libraries(test_workbench_bridge
    PRIVATE
        ui
        Qt6::Test
        GTest::GTest
)
target_compile_features(test_workbench_bridge PRIVATE cxx_std_17)
add_test(NAME WorkbenchBridge COMMAND test_workbench_bridge)

# QML 表格模型层测试 — T18
add_executable(test_qml_models test_qml_models.cpp)
target_link_libraries(test_qml_models
    PRIVATE
        ui
        Qt6::Test
        GTest::GTest
)
target_compile_features(test_qml_models PRIVATE cxx_std_17)
add_test(NAME QmlModels COMMAND test_qml_models)

# QML UI 面板测试 — T19
add_executable(test_qml_ui_panels test_qml_ui_panels.cpp)
target_link_libraries(test_qml_ui_panels
    PRIVATE
        ui
        Qt6::Test
        GTest::GTest
)
target_compile_features(test_qml_ui_panels PRIVATE cxx_std_17)
add_test(NAME QmlUiPanels COMMAND test_qml_ui_panels)

# 集成测试 — T25
add_executable(test_integration test_integration.cpp)
target_link_libraries(test_integration
    PRIVATE
        app
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_integration PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_integration PRIVATE cxx_std_17)
add_test(NAME Integration COMMAND test_integration)

# ViewManager 视图管理器测试 — T30
add_executable(test_view_manager test_view_manager.cpp)
target_link_libraries(test_view_manager
    PRIVATE
        visualization
        GTest::GTest
        GTest::Main
)
target_include_directories(test_view_manager PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_view_manager PRIVATE cxx_std_17)
add_test(NAME ViewManager COMMAND test_view_manager)

# Load 载荷数据模型测试 — T32
add_executable(test_load_model test_load_model.cpp)
target_link_libraries(test_load_model
    PRIVATE
        model
        GTest::GTest
        GTest::Main
)
target_compile_features(test_load_model PRIVATE cxx_std_17)
add_test(NAME LoadModel COMMAND test_load_model)

# LoadCase 与 LoadCombination 测试 — T33
add_executable(test_loadcase test_loadcase.cpp)
target_link_libraries(test_loadcase
    PRIVATE
        model
        GTest::GTest
        GTest::Main
)
target_compile_features(test_loadcase PRIVATE cxx_std_17)
add_test(NAME LoadCase COMMAND test_loadcase)

# DesignWorkbench 工作台测试 — T34
add_executable(test_design_workbench test_design_workbench.cpp)
target_link_libraries(test_design_workbench
    PRIVATE
        app
        GTest::GTest
        GTest::Main
)
target_include_directories(test_design_workbench PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_design_workbench PRIVATE cxx_std_17)
add_test(NAME DesignWorkbench COMMAND test_design_workbench)

# SpecWorkbench 工作台测试 — T35
add_executable(test_spec_workbench test_spec_workbench.cpp)
target_link_libraries(test_spec_workbench
    PRIVATE
        app
        GTest::GTest
        GTest::Main
)
target_include_directories(test_spec_workbench PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_spec_workbench PRIVATE cxx_std_17)
add_test(NAME SpecWorkbench COMMAND test_spec_workbench)

# OCCT→VTK 网格转换测试 — T37
add_executable(test_occt_to_vtk test_occt_to_vtk.cpp)
target_link_libraries(test_occt_to_vtk
    PRIVATE
        vtk_visualization
        geometry
        GTest::GTest
        GTest::Main
)
target_include_directories(test_occt_to_vtk PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_occt_to_vtk PRIVATE cxx_std_17)
add_test(NAME OcctToVtk COMMAND test_occt_to_vtk)

# VTK 场景管理测试 — T38
add_executable(test_vtk_scene test_vtk_scene.cpp)
target_link_libraries(test_vtk_scene
    PRIVATE
        vtk_visualization
        GTest::GTest
        GTest::Main
)
target_compile_features(test_vtk_scene PRIVATE cxx_std_17)
add_test(NAME VtkScene COMMAND test_vtk_scene)

add_executable(test_workbench_switch test_workbench_switch.cpp)
target_link_libraries(test_workbench_switch 
    PRIVATE 
        app 
        ui 
        GTest::GTest 
        GTest::Main)
add_test(NAME test_workbench_switch COMMAND test_workbench_switch)

add_executable(test_vtk_qml test_vtk_qml.cpp)
target_link_libraries(test_vtk_qml
    PRIVATE
        vtk_visualization
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
        GTest::GTest
        GTest::Main
)
target_compile_features(test_vtk_qml PRIVATE cxx_std_17)
add_test(NAME VtkQml COMMAND test_vtk_qml)

# AnalysisWorkbench 工作台测试 — T44
add_executable(test_analysis_workbench test_analysis_workbench.cpp)
target_link_libraries(test_analysis_workbench
    PRIVATE
        app
        model
        GTest::GTest
        GTest::Main
)
target_include_directories(test_analysis_workbench PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_analysis_workbench PRIVATE cxx_std_17)
add_test(NAME AnalysisWorkbench COMMAND test_analysis_workbench)

# Phase 2 端到端集成测试 — T45
add_executable(test_phase2_integration test_phase2_integration.cpp)
target_link_libraries(test_phase2_integration
    PRIVATE
        app
        engine
        visualization
        GTest::GTest
        GTest::Main
)
target_include_directories(test_phase2_integration PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_phase2_integration PRIVATE cxx_std_17)
add_test(NAME Phase2Integration COMMAND test_phase2_integration)

# Command 基类 + MacroCommand + PropertyApplier 测试 — T2
add_executable(test_command_base test_command_base.cpp)
target_link_libraries(test_command_base
    PRIVATE
        command
        model
        GTest::GTest
        GTest::Main
)
target_include_directories(test_command_base PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_command_base PRIVATE cxx_std_17)
add_test(NAME CommandBase COMMAND test_command_base)

# CommandStack 命令栈管理器测试 — T3
add_executable(test_command_stack test_command_stack.cpp)
target_link_libraries(test_command_stack
    PRIVATE
        command
        GTest::GTest
        GTest::Main
)
target_compile_features(test_command_stack PRIVATE cxx_std_17)
add_test(NAME CommandStack COMMAND test_command_stack)

# PropertyCommands 测试 — T4
add_executable(test_property_commands test_property_commands.cpp)
target_link_libraries(test_property_commands
    PRIVATE
        command
        app
        GTest::GTest
        GTest::Main
)
target_include_directories(test_property_commands PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_property_commands PRIVATE cxx_std_17)
add_test(NAME PropertyCommands COMMAND test_property_commands)

# CommandRegistry 测试 — T5
add_executable(test_command_registry test_command_registry.cpp)
target_link_libraries(test_command_registry
    PRIVATE
        command
        app
        GTest::GTest
        GTest::Main
)
target_include_directories(test_command_registry PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_command_registry PRIVATE cxx_std_17)
add_test(NAME CommandRegistry COMMAND test_command_registry)

# 结构命令测试 — T8
add_executable(test_structural_commands test_structural_commands.cpp)
target_link_libraries(test_structural_commands
    PRIVATE
        command
        app
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_structural_commands PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_structural_commands PRIVATE cxx_std_17)
add_test(NAME StructuralCommands COMMAND test_structural_commands)

# InsertComponentCommand 测试 — T9
add_executable(test_insert_component test_insert_component.cpp)
target_link_libraries(test_insert_component
    PRIVATE
        command
        app
        engine
        GTest::GTest
        GTest::Main
)
target_include_directories(test_insert_component PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_compile_features(test_insert_component PRIVATE cxx_std_17)
add_test(NAME InsertComponent COMMAND test_insert_component)
