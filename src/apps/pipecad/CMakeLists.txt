# Phase 4 / apps/pipecad — pipecad 应用目录模板
# 子域分发：T57 骨架，物理内容由 T62 迁入
# 子域对应关系：
#   model/     ← src/model/（PipePoint、Segment、Route 等领域对象）
#   engine/    ← src/engine/（BendCalculator、ComponentCatalog、各 Builder）
#   workbench/ ← src/app/ 中工作台系统（DesignWorkbench、AnalysisWorkbench 等）
#   ui/        ← src/ui/（QML 桥接层）
#   resources/ ← app 资源位（图标、QML、配置等）
# main.cpp 从 src/main.cpp 迁入，可执行目标 pipecad 定义于此。
# 复制为新 app 时，优先修改 PIPECAD_APP_ID 与 PIPECAD_QML_MAIN_FILE。

set(PIPECAD_APP_ID "pipecad")
set(PIPECAD_APP_TARGET "${PIPECAD_APP_ID}_app")
set(PIPECAD_APP_EXECUTABLE "${PIPECAD_APP_ID}")
set(PIPECAD_APP_MODEL_TARGET "${PIPECAD_APP_ID}_app_model")
set(PIPECAD_APP_ENGINE_TARGET "${PIPECAD_APP_ID}_app_engine")
set(PIPECAD_APP_WORKBENCH_TARGET "${PIPECAD_APP_ID}_app_workbench")
set(PIPECAD_APP_UI_TARGET "${PIPECAD_APP_ID}_app_ui")
set(PIPECAD_APP_RESOURCES_TARGET "${PIPECAD_APP_ID}_app_resources")

set(PIPECAD_QML_MAIN_FILE "${CMAKE_SOURCE_DIR}/ui/main.qml" CACHE FILEPATH
    "${PIPECAD_APP_ID} 应用启动 QML 文件")

add_subdirectory(model)
add_subdirectory(engine)
add_subdirectory(workbench)
add_subdirectory(ui)
add_subdirectory(resources)

add_library(${PIPECAD_APP_TARGET} STATIC
	placeholder.cpp
)

target_compile_features(${PIPECAD_APP_TARGET} PUBLIC cxx_std_17)

target_link_libraries(${PIPECAD_APP_TARGET}
	PUBLIC
		pipecad_lib
		${PIPECAD_APP_MODEL_TARGET}
		${PIPECAD_APP_ENGINE_TARGET}
		${PIPECAD_APP_WORKBENCH_TARGET}
		${PIPECAD_APP_UI_TARGET}
		${PIPECAD_APP_RESOURCES_TARGET}
		Qt6::Quick
		Qt6::Qml
)

add_executable(${PIPECAD_APP_EXECUTABLE}
	main.cpp
)

target_compile_features(${PIPECAD_APP_EXECUTABLE} PRIVATE cxx_std_17)

target_compile_definitions(${PIPECAD_APP_EXECUTABLE}
	PRIVATE
		PIPECAD_QML_MAIN="${PIPECAD_QML_MAIN_FILE}"
)

target_link_libraries(${PIPECAD_APP_EXECUTABLE}
	PRIVATE
		${PIPECAD_APP_TARGET}
)
