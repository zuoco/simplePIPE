# Phase 4 / lib::base — 基础类型层
# T58: foundation（UUID、Variant、Math、Signal、Log）已迁入
# 头文件位于 foundation/ 子目录，保持 #include "foundation/..." 路径兼容

add_library(lib_base INTERFACE)
add_library(lib::base ALIAS lib_base)

target_include_directories(lib_base INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(lib_base INTERFACE cxx_std_17)

# T63: C++20 模块接口单元（pipecad.base.math / pipecad.base.types / pipecad.base）
# 需要 CMake >= 3.28 与 GCC >= 12 / Clang >= 16 的模块支持
add_library(lib_base_modules STATIC)
add_library(lib::base::modules ALIAS lib_base_modules)

target_sources(lib_base_modules
    PUBLIC
    FILE_SET CXX_MODULES FILES
        baseMod/pipecad.base.math.cppm
        baseMod/pipecad.base.types.cppm
        baseMod/pipecad.base.cppm
)

target_include_directories(lib_base_modules
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(lib_base_modules PUBLIC cxx_std_20)

# lib_base 聚合模块目标，使所有链接 lib_base 的目标也能获得模块接口
target_link_libraries(lib_base INTERFACE lib_base_modules)
