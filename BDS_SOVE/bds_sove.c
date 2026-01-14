/*
 * bds_sove.c
 * 流动站程序源文件
 * 功能：通过互联网接受基站发送来的数据，然后发送给ttyS1
 * 代码作者：ClancyShang
 * 最后修改时间：2026-01-13
 */

#include "bds_sove.h"

/**
 * @brief 初始化串口
 * @param port 串口设备路径
 * @param baud 波特率
 * @return 成功返回文件描述符，失败返回-1
 */
int init_serial(const char *port, speed_t baud)
{
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("open serial port failed");
        return -1;
    }

    struct termios options;
    if (tcgetattr(fd, &options) != 0) {
        perror("tcgetattr failed");
        close(fd);
        return -1;
    }

    // 设置波特率
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);

    // 设置数据位
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    // 设置停止位
    options.c_cflag &= ~CSTOPB;

    // 设置奇偶校验位
    options.c_cflag &= ~PARENB;

    // 设置硬件流控
    options.c_cflag &= ~CRTSCTS;

    // 设置软件流控
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    // 设置原始模式
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    // 设置读取超时
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;

    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        perror("tcsetattr failed");
        close(fd);
        return -1;
    }

    return fd;
}

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
 * @brief 从网络接收数据并发送到串口
 * @param sock_fd socket描述符
 * @param serial_fd 串口文件描述符
 */
void network_to_serial(int sock_fd, int serial_fd)
{
    char buffer[BUFFER_SIZE];
    int bytes_received, bytes_written;
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

    while (1) {
        // 从网络接收数据
        bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            // 发送到串口
            bytes_written = write(serial_fd, buffer, bytes_received);
            if (bytes_written < 0) {
                perror("write failed");
                break;
            } else if (bytes_written != bytes_received) {
                fprintf(stderr, "write incomplete data\n");
            }
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        } else {
            perror("recv failed");
            break;
        }
    }

    close(client_fd);
}

/**
 * @brief 主函数
 * @return 成功返回0，失败返回-1
 */
int main()
{
    int serial_fd, sock_fd;

    // 初始化串口
    serial_fd = init_serial(SERIAL_PORT, BAUD_RATE);
    if (serial_fd < 0) {
        fprintf(stderr, "init_serial failed\n");
        return -1;
    }

    // 初始化服务器socket
    sock_fd = init_server_socket(LISTEN_PORT);
    if (sock_fd < 0) {
        fprintf(stderr, "init_server_socket failed\n");
        close(serial_fd);
        return -1;
    }

    printf("BDS rover station started. Listening on port %d, sending to %s\n", 
           LISTEN_PORT, SERIAL_PORT);

    // 开始数据转发
    while (1) {
        network_to_serial(sock_fd, serial_fd);
    }

    // 关闭资源
    close(serial_fd);
    close(sock_fd);

    return 0;
}
