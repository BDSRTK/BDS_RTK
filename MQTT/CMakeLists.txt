# CMakeLists.txt for MQTT client
# 代码作者：ClancyShang
# 最后修改时间：2026-01-20

cmake_minimum_required(VERSION 3.10)
project(MQTT_CLIENT)

# 继承全局输出目录设置，不覆盖

# 添加可执行文件（基于socket的简单MQTT客户端，不依赖外部库）
add_executable(simple_mqtt_client simple_mqtt_client.c)

# 不需要额外的库依赖，只需要标准C库

