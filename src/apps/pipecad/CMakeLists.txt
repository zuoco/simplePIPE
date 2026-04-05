# Phase 4 / apps/pipecad — pipecad 应用目录模板
# 子域分发：T57 骨架，物理内容由 T62 迁入
# 子域对应关系：
#   model/     ← src/model/（PipePoint、Segment、Route 等领域对象）
#   engine/    ← src/engine/（BendCalculator、ComponentCatalog、各 Builder）
#   workbench/ ← src/app/ 中工作台系统（DesignWorkbench、AnalysisWorkbench 等）
#   ui/        ← src/ui/（QML 桥接层）
# main.cpp 从 src/main.cpp 迁入，可执行目标 pipecad 定义于此

add_subdirectory(model)
add_subdirectory(engine)
add_subdirectory(workbench)
add_subdirectory(ui)

add_library(pipecad_app STATIC
	placeholder.cpp
)

target_compile_features(pipecad_app PUBLIC cxx_std_17)

target_link_libraries(pipecad_app
	PUBLIC
		pipecad_lib
		pipecad_app_model
		pipecad_app_engine
		pipecad_app_ui
		Qt6::Quick
		Qt6::Qml
)

add_executable(pipecad
	main.cpp
)

target_compile_features(pipecad PRIVATE cxx_std_17)

target_compile_definitions(pipecad
	PRIVATE
		PIPECAD_QML_MAIN="${CMAKE_SOURCE_DIR}/ui/main.qml"
)

target_link_libraries(pipecad
	PRIVATE
		pipecad_app
)
