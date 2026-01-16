/*
 * bds_base_test.c
 * 基站测试程序
 * 功能：直接发送测试数据"BASERTK_TEST"，不依赖串口和其他模块
 * 代码作者：ClancyShang
 * 最后修改时间：2026-01-16
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 网络配置
#define SERVER_IP "127.0.0.1"  // 服务器IP地址
#define SERVER_PORT 8888       // 服务器端口号

/**
 * @brief 初始化socket连接
 * @param ip 服务器IP地址
 * @param port 服务器端口号
 * @return 成功返回socket描述符，失败返回-1
 */
int init_socket(const char *ip, int port)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock_fd);
        return -1;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}

/**
 * @brief 主函数
 * @return 成功返回0，失败返回-1
 */
int main()
{
    int sock_fd;
    char *server_ip = SERVER_IP;
    const char *test_data = "BASERTK_TEST";
    int data_len = strlen(test_data);

    // 初始化网络连接
    sock_fd = init_socket(server_ip, SERVER_PORT);
    if (sock_fd < 0) {
        fprintf(stderr, "init_socket failed\n");
        return -1;
    }

    printf("BDS base station test started. Connecting to %s:%d\n", 
           server_ip, SERVER_PORT);

    // 发送测试数据
    int bytes_sent = send(sock_fd, test_data, data_len, 0);
    if (bytes_sent < 0) {
        perror("send failed");
        close(sock_fd);
        return -1;
    } else if (bytes_sent != data_len) {
        fprintf(stderr, "send incomplete data\n");
        close(sock_fd);
        return -1;
    }

    printf("Successfully sent test data: %s\n", test_data);
    printf("Sent %d bytes\n", bytes_sent);

    // 关闭资源
    close(sock_fd);

    return 0;
}
