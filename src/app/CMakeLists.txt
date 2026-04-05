# T61: app + command 已迁入 src/lib/runtime + src/lib/framework
# lib_framework 由 src/lib/framework/CMakeLists.txt 定义（在 lib 子树优先加载）
add_library(app ALIAS lib_framework)
