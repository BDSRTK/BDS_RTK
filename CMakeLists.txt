# CMakeLists.txt for BDS_RTK module
# 代码作者：ClancyShang
# 最后修改时间：2026-01-20

# 设置全局输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../../output/bin")

# 包含子目录
add_subdirectory(BDS_BASE)
add_subdirectory(BDS_SOVE)
add_subdirectory(MQTT)

