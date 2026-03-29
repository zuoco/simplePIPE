cmake_minimum_required(VERSION 3.24)
project(qml-vsg-occt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 本地预编译库 fallback (OCCT/VSG)
# 迁入 pixi 后删除这两行
list(APPEND CMAKE_PREFIX_PATH
  ${PROJECT_SOURCE_DIR}/lib/occt
  ${PROJECT_SOURCE_DIR}/lib/vsg
  ${PROJECT_SOURCE_DIR}/lib/vtk
)

# 第三方库
find_package(OpenCASCADE REQUIRED)
find_package(vsg REQUIRED)
find_package(VTK 9.6 REQUIRED COMPONENTS CommonCore CommonDataModel)
find_package(Qt6 REQUIRED COMPONENTS Quick Qml Test)
find_package(nlohmann_json REQUIRED)

# 源码
add_subdirectory(src)

# 测试
enable_testing()
add_subdirectory(tests)
