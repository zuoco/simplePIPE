# Phase 4 / apps/pipecad/engine — 管道领域引擎
# T62: src/engine/ 已物理迁入此目录；旧 target 名 engine 保留为兼容别名。

add_library(pipecad_app_engine STATIC
	BendCalculator.cpp
	RunBuilder.cpp
	BendBuilder.cpp
	ReducerBuilder.cpp
	TeeBuilder.cpp
	ValveBuilder.cpp
	FlexJointBuilder.cpp
	BeamBuilder.cpp
	AccessoryBuilder.cpp
	GeometryDeriver.cpp
	ComponentCatalog.cpp
	TopologyManager.cpp
	ConstraintSolver.cpp
	PipelineValidator.cpp
	RecomputeEngine.cpp
)

add_library(lib::apps::pipecad::engine ALIAS pipecad_app_engine)

target_include_directories(pipecad_app_engine
	PUBLIC
		${CMAKE_SOURCE_DIR}/src/apps/pipecad
		${CMAKE_SOURCE_DIR}/src
		${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(pipecad_app_engine PUBLIC cxx_std_17)

target_link_libraries(pipecad_app_engine
	PUBLIC
		pipecad_app_model
		geometry
	PRIVATE
		${OpenCASCADE_ModelingAlgorithms_LIBRARIES}
)
