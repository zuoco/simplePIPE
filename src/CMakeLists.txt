# Phase 4 / T50 规则冻结说明（仅设计约束，不改变当前构建行为）：
# 1) 统一架构库目标名固定为 pipecad_lib
# 2) 当前 app 业务库目标名固定为 pipecad_app
# 3) 当前 app 可执行目标名固定为 pipecad
# 4) 新增 app 采用 <name>_app + <name> 命名对
# 5) 本文件当前仍维持旧拓扑，命名规则将在 T53 起逐步落地

add_subdirectory(foundation)
add_subdirectory(geometry)
add_subdirectory(model)
add_subdirectory(engine)
add_subdirectory(visualization)
add_subdirectory(vtk-visualization)
add_subdirectory(app)
add_subdirectory(command)  # INTERFACE 别名，必须在 app 之后
add_subdirectory(ui)

# T53: pipecad_lib 聚合架构目标（INTERFACE，M1 过渡版）
# 汇聚全部架构子库，供未来 apps/pipecad 业务库统一链接。
# 通过 app 传递引入：foundation, geometry, model, engine,
#   vtk_visualization, visualization, command 源码, RecomputeEngine.cpp
# T54 后 app 不再吞并 RecomputeEngine.cpp，但 pipecad_lib 拓扑无需修改。
# T55 后引入 pipecad_app（业务库）与 pipecad（可执行）两个正式目标。
# T75 后清理旧兼容别名（foundation, geometry, model, engine 等旧 target 名）。
add_library(pipecad_lib INTERFACE)
target_link_libraries(pipecad_lib INTERFACE app)

add_executable(pipecad_app
	main.cpp
)

target_compile_features(pipecad_app PRIVATE cxx_std_17)

target_compile_definitions(pipecad_app
	PRIVATE
		PIPECAD_QML_MAIN="${CMAKE_SOURCE_DIR}/ui/main.qml"
)

target_link_libraries(pipecad_app
	PRIVATE
		ui
		Qt6::Quick
		Qt6::Qml
)
