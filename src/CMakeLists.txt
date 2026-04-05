# Phase 4 / T50 规则冻结说明：
# 1) 统一架构库目标名固定为 pipecad_lib
# 2) 当前 app 业务库目标名固定为 pipecad_app
# 3) 当前 app 可执行目标名固定为 pipecad
# 4) 新增 app 采用 <name>_app + <name> 命名对

# T75: 旧目录兼容层已清理——所有旧目录的 CMakeLists.txt 已删除，
#       别名定义统一迁移至此处。src/lib 与 src/apps 为唯一真实结构。

# Step 1: 加载统一架构库（lib_base, lib_platform_*, lib_runtime, lib_framework）
add_subdirectory(lib)

# ── 旧目录兼容别名（T75: 从各旧目录 CMakeLists.txt 迁移至此）──────────────────

# T58: foundation → lib_base
add_library(foundation ALIAS lib_base)

# T59: geometry → lib_platform_occt
add_library(geometry ALIAS lib_platform_occt)

# T60: visualization → lib_platform_vsg
add_library(visualization ALIAS lib_platform_vsg)

# T61: app → lib_framework
add_library(app ALIAS lib_framework)

# command: INTERFACE 目标，透传 app 依赖（保持向后兼容）
add_library(command INTERFACE)
target_link_libraries(command INTERFACE app)

# vtk_visualization: VtkViewport Qt 集成层（算法层已在 lib_platform_vtk）
# T75+: 源文件已迁入 src/lib/platform/vtk/vtk-visualization/
add_library(vtk_visualization STATIC
    ${CMAKE_SOURCE_DIR}/src/lib/platform/vtk/vtk-visualization/VtkViewport.cpp
)
target_include_directories(vtk_visualization
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/lib/platform/vtk
        ${CMAKE_SOURCE_DIR}/src
        ${OpenCASCADE_INCLUDE_DIR}
)
target_compile_features(vtk_visualization PUBLIC cxx_std_17)
target_link_libraries(vtk_visualization
    PUBLIC
        lib_platform_vtk
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
)

# ─────────────────────────────────────────────────────────────────────────────

# T53: pipecad_lib 聚合架构目标（INTERFACE）
# pipecad_lib 必须在 apps 之前定义，因为 apps/pipecad/ui 依赖它
add_library(pipecad_lib INTERFACE)
target_link_libraries(pipecad_lib INTERFACE app)

# Step 2: 加载业务应用目录
add_subdirectory(apps)

# ── 业务层旧目录兼容别名（需在 apps 加载后定义）─────────────────────────────

# T62: model → pipecad_app_model
add_library(model ALIAS pipecad_app_model)

# T62: engine → pipecad_app_engine
add_library(engine ALIAS pipecad_app_engine)

# T62: ui → pipecad_app_ui
add_library(ui ALIAS pipecad_app_ui)
