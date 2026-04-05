# Phase 4 / apps/pipecad/workbench — 工作台系统
# 目标：DesignWorkbench、SpecWorkbench、AnalysisWorkbench、WorkbenchManager（来自 src/app/）迁入时间线：T62

add_library(pipecad_app_workbench STATIC
	placeholder.cpp
)

add_library(lib::apps::pipecad::workbench ALIAS pipecad_app_workbench)

target_include_directories(pipecad_app_workbench
	PUBLIC
		${CMAKE_SOURCE_DIR}/src/apps/pipecad
		${CMAKE_SOURCE_DIR}/src
)

target_compile_features(pipecad_app_workbench PUBLIC cxx_std_17)

target_link_libraries(pipecad_app_workbench
	PUBLIC
		pipecad_lib
)
