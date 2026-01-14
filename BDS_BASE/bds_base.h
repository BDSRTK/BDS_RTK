/*
 * bds_base.h
 * 基站程序头文件
 * 功能：定义常量、结构体和函数声明
 * 代码作者：ClancyShang
 * 最后修改时间：2026-01-13
 */

#ifndef BDS_BASE_H
#define BDS_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>

// 串口配置
#define SERIAL_PORT "/dev/ttyS1"
#define BAUD_RATE B115200

// 网络配置
#define SERVER_IP "127.0.0.1"  // 服务器IP地址，实际使用时需要修改
#define SERVER_PORT 8888       // 服务器端口号
#define BUFFER_SIZE 1024       // 缓冲区大小

// 函数声明
int init_serial(const char *port, speed_t baud);
int init_socket(const char *ip, int port);
void serial_to_network(int serial_fd, int sock_fd);
char *get_local_ip(const char *ifname);

#endif /* BDS_BASE_H */
