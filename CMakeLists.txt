cmake_minimum_required(VERSION 3.24)
# License: Apache-2.0 (see LICENSE file)
project(qml-vsg-occt VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 默认采用稳定构建策略：Ninja 下所有链接规则统一串行，避免链接阶段资源峰值打满开发机。
if(CMAKE_GENERATOR MATCHES "Ninja")
  set_property(GLOBAL PROPERTY JOB_POOLS link_pool=1)
  set(CMAKE_JOB_POOL_LINK link_pool)
endif()

# 本地预编译库 fallback (OCCT/VSG)
# VTK 已迁入 pixi conda-forge，仅保留 OCCT/VSG 本地路径
list(APPEND CMAKE_PREFIX_PATH
  ${PROJECT_SOURCE_DIR}/lib/occt
  ${PROJECT_SOURCE_DIR}/lib/vsg
)

# 第三方库
find_package(OpenCASCADE REQUIRED)
find_package(vsg REQUIRED)
find_package(Freetype REQUIRED)
find_package(VTK 9.6 COMPONENTS CommonCore CommonDataModel RenderingCore)
find_package(Qt6 REQUIRED COMPONENTS Quick Qml Test)
find_package(nlohmann_json REQUIRED)

# 源码
add_subdirectory(src)

# 测试
enable_testing()
add_subdirectory(tests)
