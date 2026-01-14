# CMakeLists.txt for BDS_BASE module
# 代码作者：ClancyShang
# 最后修改时间：2026-01-13

# 添加可执行文件
add_executable(bds_base bds_base.c)

# 链接必要的库
target_link_libraries(bds_base m)
