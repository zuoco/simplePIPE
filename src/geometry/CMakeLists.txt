# T59: geometry 已迁入 src/lib/platform/occt，此处提供向后兼容别名
# lib_platform_occt 由 src/lib/platform/occt/CMakeLists.txt 定义（在 lib 子树优先加载）
add_library(geometry ALIAS lib_platform_occt)
