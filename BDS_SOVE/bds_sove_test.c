/*
 * bds_sove_test.c
 * 流动站测试程序
 * 功能：接收网络数据并打印，不依赖串口和其他模块
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
#define LISTEN_PORT 8888       // 监听端口号
#define BUFFER_SIZE 1024       // 缓冲区大小

/**
 * @brief 初始化服务器socket
 * @param port 监听端口号
 * @return 成功返回socket描述符，失败返回-1
 */
int init_server_socket(int port)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket creation failed");
        return -1;
    }

    // 设置SO_REUSEADDR选项，允许端口复用
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(sock_fd);
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sock_fd);
        return -1;
    }

    if (listen(sock_fd, 5) < 0) {
        perror("listen failed");
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}

/**
 * @brief 从网络接收数据并打印
 * @param sock_fd socket描述符
 */
void network_to_print(int sock_fd)
{
    char buffer[BUFFER_SIZE];
    int bytes_received;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // 接受客户端连接
    int client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept failed");
        return;
    }

    printf("Client connected: %s:%d\n", 
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 接收数据并打印
    bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';  // 添加字符串结束符
        printf("Received data: %s\n", buffer);
        printf("Received %d bytes\n", bytes_received);
    } else if (bytes_received == 0) {
        printf("Client disconnected\n");
    } else {
        perror("recv failed");
    }

    close(client_fd);
}

/**
 * @brief 主函数
 * @return 成功返回0，失败返回-1
 */
int main()
{
    int sock_fd;

    // 初始化服务器socket
    sock_fd = init_server_socket(LISTEN_PORT);
    if (sock_fd < 0) {
        fprintf(stderr, "init_server_socket failed\n");
        return -1;
    }

    printf("BDS rover station test started. Listening on port %d\n", 
           LISTEN_PORT);
    printf("Waiting for test data...\n");

    // 接收数据并打印
    network_to_print(sock_fd);

    // 关闭资源
    close(sock_fd);

    return 0;
}
