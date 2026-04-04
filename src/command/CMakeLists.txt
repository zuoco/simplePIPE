# command 层源文件已合并到 app 静态库中，此处提供 INTERFACE 别名以保持向后兼容
add_library(command INTERFACE)
target_link_libraries(command INTERFACE app)
