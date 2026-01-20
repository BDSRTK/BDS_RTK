/*
 * mqtt_test.c
 * MQTT客户端测试程序
 * 功能：连接MQTT服务器并发送"BDS-RTKtest"消息
 * 代码作者：ClancyShang
 * 最后修改时间：2026-01-20
 */

#include "mqtt_client.h"

/**
 * @brief 主函数
 * @return 成功返回0，失败返回-1
 */
int main()
{
    MQTTClient client = NULL;
    int rc = 0;

    // 创建MQTT客户端
    client = create_mqtt_client();
    if (client == NULL) {
        fprintf(stderr, "Failed to create MQTT client\n");
        return -1;
    }

    // 连接到MQTT服务器
    rc = connect_mqtt_client(client);
    if (rc != 0) {
        fprintf(stderr, "Failed to connect to MQTT server\n");
        MQTTClient_destroy(&client);
        return -1;
    }

    // 发布测试消息
    const char *test_message = "BDS-RTKtest";
    rc = publish_mqtt_message(client, test_message);
    if (rc != 0) {
        fprintf(stderr, "Failed to publish message\n");
        disconnect_mqtt_client(client);
        return -1;
    }

    // 断开连接
    rc = disconnect_mqtt_client(client);
    if (rc != 0) {
        fprintf(stderr, "Failed to disconnect from MQTT server\n");
        return -1;
    }

    printf("MQTT test completed successfully\n");
    return 0;
}
