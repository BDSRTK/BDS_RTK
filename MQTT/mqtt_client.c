/*
 * mqtt_client.c
 * MQTT客户端源文件
 * 功能：实现MQTT客户端的连接、发布等功能
 * 代码作者：ClancyShang
 * 最后修改时间：2026-01-20
 */

#include "mqtt_client.h"

/**
 * @brief 创建MQTT客户端
 * @return 成功返回MQTT客户端句柄，失败返回NULL
 */
MQTTClient create_mqtt_client()
{
    MQTTClient client;
    int rc;

    // 创建MQTT客户端
    rc = MQTTClient_create(&client, MQTT_SERVER, MQTT_CLIENT_ID, 
                          MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_create failed: %d\n", rc);
        return NULL;
    }

    return client;
}

/**
 * @brief 连接MQTT服务器
 * @param client MQTT客户端句柄
 * @return 成功返回0，失败返回-1
 */
int connect_mqtt_client(MQTTClient client)
{
    if (client == NULL) {
        fprintf(stderr, "Invalid MQTT client\n");
        return -1;
    }

    int rc;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    // 设置连接选项
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = MQTT_USERNAME;
    conn_opts.password = MQTT_PASSWORD;

    // 连接到MQTT服务器
    rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_connect failed: %d\n", rc);
        return -1;
    }

    printf("Connected to MQTT server: %s\n", MQTT_SERVER);
    return 0;
}

/**
 * @brief 发布MQTT消息
 * @param client MQTT客户端句柄
 * @param message 要发布的消息
 * @return 成功返回0，失败返回-1
 */
int publish_mqtt_message(MQTTClient client, const char *message)
{
    if (client == NULL || message == NULL) {
        fprintf(stderr, "Invalid parameters\n");
        return -1;
    }

    int rc;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;

    // 设置发布消息
    pubmsg.payload = (void *)message;
    pubmsg.payloadlen = strlen(message);
    pubmsg.qos = MQTT_QOS;
    pubmsg.retained = 0;

    MQTTClient_deliveryToken token;

    // 发布消息
    rc = MQTTClient_publishMessage(client, MQTT_TOPIC, &pubmsg, &token);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_publishMessage failed: %d\n", rc);
        return -1;
    }

    // 等待消息发布完成
    rc = MQTTClient_waitForCompletion(client, token, 10000L);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_waitForCompletion failed: %d\n", rc);
        return -1;
    }

    printf("Message published to topic %s: %s\n", MQTT_TOPIC, message);
    return 0;
}

/**
 * @brief 断开MQTT连接
 * @param client MQTT客户端句柄
 * @return 成功返回0，失败返回-1
 */
int disconnect_mqtt_client(MQTTClient client)
{
    if (client == NULL) {
        fprintf(stderr, "Invalid MQTT client\n");
        return -1;
    }

    int rc;

    // 断开连接
    rc = MQTTClient_disconnect(client, 10000L);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_disconnect failed: %d\n", rc);
        return -1;
    }

    // 销毁客户端
    MQTTClient_destroy(&client);

    printf("Disconnected from MQTT server\n");
    return 0;
}
