# Phase 4 / lib::base — 基础类型层
# T58: foundation（UUID、Variant、Math、Signal、Log）已迁入
# 头文件位于 foundation/ 子目录，保持 #include "foundation/..." 路径兼容

add_library(lib_base INTERFACE)
add_library(lib::base ALIAS lib_base)

target_include_directories(lib_base INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(lib_base INTERFACE cxx_std_17)
