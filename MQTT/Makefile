# Makefile for simple MQTT client
# 代码作者：ClancyShang
# 最后修改时间：2026-01-20

# 使用项目统一的交叉编译工具链
TOOL_CHAIN_PATH = /opt/gcc-ubuntu-9.3.0-2020.03-x86_64-aarch64-linux-gnu/bin/
TOOLCHAIN_PREFIX = aarch64-linux-gnu-
CC = $(TOOL_CHAIN_PATH)$(TOOLCHAIN_PREFIX)gcc
CFLAGS = -Wall -g
TARGET = simple_mqtt_client
SRCS = simple_mqtt_client.c
OBJS = $(SRCS:.c=.o)

# 设置输出目录
OUT_DIR = ../OUT

.PHONY: all clean

all: $(OUT_DIR)/$(TARGET)

$(OUT_DIR)/$(TARGET): $(OBJS)
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $(OUT_DIR)/$(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(OUT_DIR)/$(TARGET)
