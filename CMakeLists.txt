# CMakeLists.txt for BDS_RTK project
# 代码作者：ClancyShang
# 最后修改时间：2026-01-13

cmake_minimum_required(VERSION 3.10)
project(BDS_RTK)

# 设置可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/OUT)

# 添加子目录
add_subdirectory(BDS_BASE)
add_subdirectory(BDS_SOVE)
