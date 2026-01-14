/*
 * bds_sove.h
 * 流动站程序头文件
 * 功能：定义常量、结构体和函数声明
 * 代码作者：ClancyShang
 * 最后修改时间：2026-01-13
 */

#ifndef BDS_SOVE_H
#define BDS_SOVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 串口配置
#define SERIAL_PORT "/dev/ttyS1"
#define BAUD_RATE B115200

// 网络配置
#define LISTEN_PORT 8888       // 监听端口号
#define BUFFER_SIZE 1024       // 缓冲区大小

// 函数声明
int init_serial(const char *port, speed_t baud);
int init_server_socket(int port);
void network_to_serial(int sock_fd, int serial_fd);

#endif /* BDS_SOVE_H */
