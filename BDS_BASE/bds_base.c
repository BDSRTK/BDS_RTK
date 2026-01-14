/*
 * bds_base.c
 * 基站程序源文件
 * 功能：从ttyS1串口接收原始数据，通过互联网发送出去
 * 代码作者：ClancyShang
 * 最后修改时间：2026-01-13
 */

#include "bds_base.h"

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
 * @brief 初始化网络连接
 * @param ip 服务器IP地址
 * @param port 服务器端口号
 * @return 成功返回socket描述符，失败返回-1
 */
/**
 * @brief 获取本地IP地址
 * @param ifname 网卡名称，如"eth0"、"wlan0"等，如果为NULL则获取第一个可用的非回环IPv4地址
 * @return 成功返回IP地址字符串，失败返回NULL
 */
char *get_local_ip(const char *ifname)
{
    struct ifaddrs *ifaddr, *ifa;
    int family;
    char *ip = NULL;

    // 获取所有网络接口
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        return NULL;
    }

    printf("Available network interfaces:\n");
    
    // 遍历所有网络接口
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        family = ifa->ifa_addr->sa_family;
        
        // 打印所有网络接口名称
        printf("  - %s (family: %d)\n", ifa->ifa_name, family);

        // 只处理IPv4地址
        if (family == AF_INET) {
            char temp_ip[INET_ADDRSTRLEN];
            inet_ntop(family, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, temp_ip, INET_ADDRSTRLEN);
            printf("    IPv4 address: %s\n", temp_ip);
            
            // 排除回环地址（127.0.0.1）
            if (strcmp(temp_ip, "127.0.0.1") == 0) {
                continue;
            }
            
            // 如果指定了网卡名称，则只匹配该网卡
            // 否则，返回第一个找到的非回环IPv4地址
            if ((ifname == NULL) || (strcmp(ifa->ifa_name, ifname) == 0)) {
                ip = malloc(INET_ADDRSTRLEN);
                if (ip == NULL) {
                    perror("malloc failed");
                    freeifaddrs(ifaddr);
                    return NULL;
                }
                strcpy(ip, temp_ip);
                break;
            }
        }
    }

    // 如果没有找到非回环地址，尝试返回回环地址
    if (ip == NULL) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL) {
                continue;
            }

            family = ifa->ifa_addr->sa_family;
            
            if (family == AF_INET) {
                char temp_ip[INET_ADDRSTRLEN];
                inet_ntop(family, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, temp_ip, INET_ADDRSTRLEN);
                
                // 返回回环地址
                if (strcmp(temp_ip, "127.0.0.1") == 0) {
                    ip = malloc(INET_ADDRSTRLEN);
                    if (ip == NULL) {
                        perror("malloc failed");
                        freeifaddrs(ifaddr);
                        return NULL;
                    }
                    strcpy(ip, temp_ip);
                    break;
                }
            }
        }
    }

    // 释放资源
    freeifaddrs(ifaddr);

    return ip;
}

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
 * @brief 从串口读取数据并通过网络发送
 * @param serial_fd 串口文件描述符
 * @param sock_fd socket描述符
 */
void serial_to_network(int serial_fd, int sock_fd)
{
    char buffer[BUFFER_SIZE];
    int bytes_read, bytes_sent;

    while (1) {
        // 从串口读取数据
        bytes_read = read(serial_fd, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            // 通过网络发送数据
            bytes_sent = send(sock_fd, buffer, bytes_read, 0);
            if (bytes_sent < 0) {
                perror("send failed");
                break;
            } else if (bytes_sent != bytes_read) {
                fprintf(stderr, "send incomplete data\n");
            }
        } else if (bytes_read < 0) {
            perror("read failed");
            break;
        }
    }
}

/**
 * @brief 主函数
 * @return 成功返回0，失败返回-1
 */
int main()
{
    int serial_fd, sock_fd;
    char *local_ip = NULL;
    char *server_ip = SERVER_IP;
    
    // 自动获取本地IP地址
    // 首先尝试获取任何可用的IPv4地址
    local_ip = get_local_ip(NULL);
    if (local_ip != NULL) {
        printf("Local IP address: %s\n", local_ip);
        free(local_ip);
    } else {
        printf("Warning: Failed to get local IP address\n");
    }

    // 初始化串口
    serial_fd = init_serial(SERIAL_PORT, BAUD_RATE);
    if (serial_fd < 0) {
        fprintf(stderr, "init_serial failed\n");
        return -1;
    }

    // 初始化网络连接
    sock_fd = init_socket(server_ip, SERVER_PORT);
    if (sock_fd < 0) {
        fprintf(stderr, "init_socket failed\n");
        close(serial_fd);
        return -1;
    }

    printf("BDS base station started. Listening on %s, connecting to %s:%d\n", 
           SERIAL_PORT, server_ip, SERVER_PORT);

    // 开始数据转发
    serial_to_network(serial_fd, sock_fd);

    // 关闭资源
    close(serial_fd);
    close(sock_fd);

    return 0;
}
