# Phase 4 / apps/pipecad/model — 管道领域模型
# T62: src/model/ 已物理迁入此目录；旧 target 名 model 保留为兼容别名。

add_library(pipecad_app_model STATIC
	placeholder.cpp
)

add_library(lib::apps::pipecad::model ALIAS pipecad_app_model)

target_include_directories(pipecad_app_model
	PUBLIC
		${CMAKE_SOURCE_DIR}/src/apps/pipecad
		${CMAKE_SOURCE_DIR}/src
		${OpenCASCADE_INCLUDE_DIR}
)

target_compile_features(pipecad_app_model PUBLIC cxx_std_17)

target_link_libraries(pipecad_app_model
	PUBLIC
		lib::base
	PRIVATE
		${OpenCASCADE_FoundationClasses_LIBRARIES}
		${OpenCASCADE_ModelingData_LIBRARIES}
)
