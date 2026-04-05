# T58: foundation 已迁移至 src/lib/base/foundation/
# 此目标为向后兼容别名，确保旧链接代码（geometry、model、engine、tests 等）不需改动
# lib_base 由 src/lib/base/CMakeLists.txt 定义（add_subdirectory(lib) 必须先于本文件执行）
add_library(foundation ALIAS lib_base)
