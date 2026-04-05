# Phase 4 / src/apps — 应用目录根
# T76: 固化 app 注册模板。新增 app 时仅需：
# 1) 复制 src/apps/pipecad 为 src/apps/<name>
# 2) 将 <name> 追加到 PIPECAD_APP_LIST

set(PIPECAD_APP_LIST "pipecad" CACHE STRING
	"需要构建的 app 列表（分号分隔），例如：pipecad;viewer")

function(pipecad_register_app app_name)
	set(app_dir "${CMAKE_CURRENT_SOURCE_DIR}/${app_name}")

	if(NOT EXISTS "${app_dir}/CMakeLists.txt")
		message(FATAL_ERROR "[apps] 未找到 ${app_name}/CMakeLists.txt")
	endif()

	foreach(required_entry model engine workbench ui resources main.cpp)
		if(NOT EXISTS "${app_dir}/${required_entry}")
			message(FATAL_ERROR
				"[apps] ${app_name} 缺少模板骨架条目: ${required_entry}")
		endif()
	endforeach()

	add_subdirectory(${app_name})
endfunction()

foreach(app_name IN LISTS PIPECAD_APP_LIST)
	pipecad_register_app(${app_name})
endforeach()
