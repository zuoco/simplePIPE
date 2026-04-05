# T60: visualization 已迁入 src/lib/platform/vsg，此处提供向后兼容别名
# lib_platform_vsg 由 src/lib/platform/vsg/CMakeLists.txt 定义（在 lib 子树优先加载）
add_library(visualization ALIAS lib_platform_vsg)
