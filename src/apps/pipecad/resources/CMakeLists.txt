# Phase 4 / apps/pipecad/resources — app 资源位
# 资源位用于承载图标、QML、配置和后续 qrc 打包入口。

add_library(pipecad_app_resources INTERFACE)
add_library(lib::apps::pipecad::resources ALIAS pipecad_app_resources)
