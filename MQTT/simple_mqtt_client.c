/*
 * simple_mqtt_client.c
 * 简单MQTT客户端源文件
 * 功能：使用socket实现基本的MQTT连接和发布功能，不需要外部库
 * 代码作者：ClancyShang
 * 最后修改时间：2026-01-20
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// MQTT服务器配置
#define MQTT_SERVER      "www.bjfzkj.com.cn"
#define MQTT_PORT        1883
#define MQTT_CLIENT_ID   "bds_rtk_client"
#define MQTT_USERNAME    "mqttgnss"
#define MQTT_PASSWORD    "feizhou@500127"
#define MQTT_TOPIC       "BDS-RTK/test"

// MQTT固定头标志位
#define MQTT_CONNECT     1   // 连接请求
#define MQTT_PUBLISH     3   // 发布消息
#define MQTT_CONNACK     2   // 连接确认

// 连接返回码
#define CONNACK_ACCEPTED 0   // 连接成功

/**
 * @brief 计算MQTT消息长度编码
 * @param length 消息长度
 * @param buffer 存储编码后的长度
 * @return 编码后的长度字节数
 */
int mqtt_encode_length(int length, unsigned char *buffer)
{
    int i = 0;
    do {
        unsigned char byte = length % 128;
        length = length / 128;
        if (length > 0) {
            byte |= 0x80;
        }
        buffer[i++] = byte;
    } while (length > 0 && i < 4);
    return i;
}

/**
 * @brief 创建MQTT连接数据包
 * @param buffer 存储连接数据包
 * @param client_id 客户端ID
 * @param username 用户名
 * @param password 密码
 * @return 连接数据包长度
 */
int mqtt_create_connect_packet(unsigned char *buffer, const char *client_id, const char *username, const char *password)
{
    int pos = 0;
    int remaining_length = 0;
    
    // 固定头
    buffer[pos++] = MQTT_CONNECT << 4;  // 消息类型
    
    // 可变头和有效载荷
    int var_pos = pos;
    
    // 协议名（MQTT）
    buffer[var_pos++] = 0x00; buffer[var_pos++] = 0x04;  // 长度
    buffer[var_pos++] = 'M'; buffer[var_pos++] = 'Q'; buffer[var_pos++] = 'T'; buffer[var_pos++] = 'T';
    
    // 协议级别（MQTT 3.1.1）
    buffer[var_pos++] = 0x04;
    
    // 连接标志
    buffer[var_pos++] = 0xC0;  // 用户名和密码标志
    
    // 保持连接时间
    buffer[var_pos++] = 0x00; buffer[var_pos++] = 0x14;  // 20秒
    
    // 客户端ID
    int client_id_len = strlen(client_id);
    buffer[var_pos++] = (client_id_len >> 8) & 0xFF;
    buffer[var_pos++] = client_id_len & 0xFF;
    memcpy(&buffer[var_pos], client_id, client_id_len);
    var_pos += client_id_len;
    
    // 用户名
    int username_len = strlen(username);
    buffer[var_pos++] = (username_len >> 8) & 0xFF;
    buffer[var_pos++] = username_len & 0xFF;
    memcpy(&buffer[var_pos], username, username_len);
    var_pos += username_len;
    
    // 密码
    int password_len = strlen(password);
    buffer[var_pos++] = (password_len >> 8) & 0xFF;
    buffer[var_pos++] = password_len & 0xFF;
    memcpy(&buffer[var_pos], password, password_len);
    var_pos += password_len;
    
    // 计算剩余长度
    remaining_length = var_pos - pos;
    
    // 编码剩余长度
    unsigned char length_buf[4];
    int length_len = mqtt_encode_length(remaining_length, length_buf);
    
    // 插入编码后的剩余长度
    memmove(&buffer[pos + length_len], &buffer[var_pos - remaining_length], remaining_length);
    memcpy(&buffer[pos], length_buf, length_len);
    
    return pos + length_len + remaining_length;
}

/**
 * @brief 创建MQTT发布数据包
 * @param buffer 存储发布数据包
 * @param topic 主题
 * @param message 消息内容
 * @return 发布数据包长度
 */
int mqtt_create_publish_packet(unsigned char *buffer, const char *topic, const char *message)
{
    int pos = 0;
    int remaining_length = 0;
    
    // 固定头
    buffer[pos++] = MQTT_PUBLISH << 4;  // 消息类型
    
    // 可变头和有效载荷
    int var_pos = pos;
    
    // 主题
    int topic_len = strlen(topic);
    buffer[var_pos++] = (topic_len >> 8) & 0xFF;
    buffer[var_pos++] = topic_len & 0xFF;
    memcpy(&buffer[var_pos], topic, topic_len);
    var_pos += topic_len;
    
    // 消息ID（QoS 0不需要）
    
    // 消息内容
    int message_len = strlen(message);
    memcpy(&buffer[var_pos], message, message_len);
    var_pos += message_len;
    
    // 计算剩余长度
    remaining_length = var_pos - pos;
    
    // 编码剩余长度
    unsigned char length_buf[4];
    int length_len = mqtt_encode_length(remaining_length, length_buf);
    
    // 插入编码后的剩余长度
    memmove(&buffer[pos + length_len], &buffer[var_pos - remaining_length], remaining_length);
    memcpy(&buffer[pos], length_buf, length_len);
    
    return pos + length_len + remaining_length;
}

/**
 * @brief 连接到MQTT服务器
 * @param server 服务器地址
 * @param port 端口号
 * @return 成功返回socket描述符，失败返回-1
 */
int connect_to_mqtt_server(const char *server, int port)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket creation failed");
        return -1;
    }
    
    // 获取服务器地址信息
    struct hostent *host = gethostbyname(server);
    if (host == NULL) {
        fprintf(stderr, "Failed to resolve host: %s\n", server);
        close(sock_fd);
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);
    
    // 连接到服务器
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sock_fd);
        return -1;
    }
    
    printf("Connected to MQTT server: %s:%d\n", server, port);
    return sock_fd;
}

/**
 * @brief 发送MQTT连接请求
 * @param sock_fd socket描述符
 * @return 成功返回0，失败返回-1
 */
int send_mqtt_connect(int sock_fd)
{
    unsigned char buffer[1024];
    int packet_len = mqtt_create_connect_packet(buffer, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
    
    int bytes_sent = send(sock_fd, buffer, packet_len, 0);
    if (bytes_sent < 0) {
        perror("send connect packet failed");
        return -1;
    }
    
    printf("Sent MQTT connect packet (%d bytes)\n", bytes_sent);
    
    // 接收连接确认
    unsigned char connack_buf[10];
    int bytes_received = recv(sock_fd, connack_buf, sizeof(connack_buf), 0);
    if (bytes_received < 0) {
        perror("recv connack failed");
        return -1;
    }
    
    // 检查连接确认
    if (bytes_received >= 4 && (connack_buf[0] & 0xF0) == (MQTT_CONNACK << 4)) {
        if (connack_buf[3] == CONNACK_ACCEPTED) {
            printf("MQTT connection accepted\n");
            return 0;
        } else {
            fprintf(stderr, "MQTT connection rejected with code: %d\n", connack_buf[3]);
            return -1;
        }
    }
    
    fprintf(stderr, "Invalid connack packet\n");
    return -1;
}

/**
 * @brief 发送MQTT发布消息
 * @param sock_fd socket描述符
 * @param message 要发布的消息
 * @return 成功返回0，失败返回-1
 */
int send_mqtt_publish(int sock_fd, const char *message)
{
    unsigned char buffer[1024];
    int packet_len = mqtt_create_publish_packet(buffer, MQTT_TOPIC, message);
    
    int bytes_sent = send(sock_fd, buffer, packet_len, 0);
    if (bytes_sent < 0) {
        perror("send publish packet failed");
        return -1;
    }
    
    printf("Sent MQTT publish packet (%d bytes)\n", bytes_sent);
    printf("Published message to topic %s: %s\n", MQTT_TOPIC, message);
    return 0;
}

/**
 * @brief 主函数
 * @return 成功返回0，失败返回-1
 */
int main()
{
    int sock_fd = -1;
    int rc = 0;
    int send_count = 0;
    
    // 连接到MQTT服务器
    sock_fd = connect_to_mqtt_server(MQTT_SERVER, MQTT_PORT);
    if (sock_fd < 0) {
        fprintf(stderr, "Failed to connect to MQTT server\n");
        return -1;
    }
    
    // 发送MQTT连接请求
    rc = send_mqtt_connect(sock_fd);
    if (rc != 0) {
        fprintf(stderr, "Failed to send MQTT connect\n");
        close(sock_fd);
        return -1;
    }
    
    // 循环发送测试消息，每秒一次
    const char *test_message = "BDS-RTKtest";
    while (send_count < 5) {  // 发送5次后退出
        rc = send_mqtt_publish(sock_fd, test_message);
        if (rc != 0) {
            fprintf(stderr, "Failed to send MQTT publish\n");
            close(sock_fd);
            return -1;
        }
        
        send_count++;
        printf("Sent message %d times\n", send_count);
        
        // 等待1秒
        sleep(1);
    }
    
    // 断开连接
    close(sock_fd);
    
    printf("MQTT test completed successfully. Sent message %d times\n", send_count);
    return 0;
}
