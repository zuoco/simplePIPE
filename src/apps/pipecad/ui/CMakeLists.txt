# Phase 4 / apps/pipecad/ui — QML 桥接层
# T62: src/ui/ 已物理迁入此目录；旧 target 名 ui 保留为兼容别名。

add_library(pipecad_app_ui STATIC
	placeholder.cpp
	VsgQuickItem.cpp
	AppController.cpp
	WorkbenchController.cpp
	PipePointTableModel.cpp
	SegmentTreeModel.cpp
	PropertyModel.cpp
	PipeSpecModel.cpp
)

add_library(lib::apps::pipecad::ui ALIAS pipecad_app_ui)

target_include_directories(pipecad_app_ui
	PUBLIC
		${CMAKE_SOURCE_DIR}/src/apps/pipecad
		${CMAKE_SOURCE_DIR}/src
)

target_compile_features(pipecad_app_ui PUBLIC cxx_std_17)

target_link_libraries(pipecad_app_ui
	PUBLIC
		pipecad_lib
		Qt6::Quick
		Qt6::Qml
)
