/*
 * mqtt_client.h
 * MQTT客户端头文件
 * 功能：定义MQTT客户端的常量、结构体和函数声明
 * 代码作者：ClancyShang
 * 最后修改时间：2026-01-20
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <MQTTClient.h>

// MQTT服务器配置
#define MQTT_SERVER      "tcp://bjfzkj.com.cn:1883"
#define MQTT_CLIENT_ID   "bds_rtk_client"
#define MQTT_USERNAME    "mqttgnss"
#define MQTT_PASSWORD    "feizhou@500127"
#define MQTT_TOPIC       "BDS-RTK/test"
#define MQTT_QOS         1

// 函数声明
MQTTClient create_mqtt_client();
int connect_mqtt_client(MQTTClient client);
int publish_mqtt_message(MQTTClient client, const char *message);
int disconnect_mqtt_client(MQTTClient client);

#endif /* MQTT_CLIENT_H */
