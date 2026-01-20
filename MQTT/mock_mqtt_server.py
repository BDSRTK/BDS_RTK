#!/usr/bin/env python3

"""
模拟MQTT服务器
功能：接收MQTT客户端连接和发布的消息
代码作者：ClancyShang
最后修改时间：2026-01-20
"""

import socket
import struct
import threading

# MQTT消息类型
MQTT_CONNECT = 1
MQTT_PUBLISH = 3
MQTT_CONNACK = 2

# 连接返回码
CONNACK_ACCEPTED = 0

class MockMQTTServer:
    def __init__(self, host='0.0.0.0', port=1883):
        self.host = host
        self.port = port
        self.server_socket = None
        self.is_running = False
        
    def start(self):
        """启动模拟MQTT服务器"""
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(5)
        self.is_running = True
        
        print(f"Mock MQTT Server started on {self.host}:{self.port}")
        print("Waiting for client connections...")
        
        try:
            while self.is_running:
                client_socket, client_addr = self.server_socket.accept()
                print(f"Client connected: {client_addr}")
                
                # 为每个客户端创建一个线程处理
                client_thread = threading.Thread(target=self.handle_client, args=(client_socket, client_addr))
                client_thread.daemon = True
                client_thread.start()
        except KeyboardInterrupt:
            print("\nServer stopped by user")
        finally:
            self.stop()
    
    def stop(self):
        """停止模拟MQTT服务器"""
        self.is_running = False
        if self.server_socket:
            self.server_socket.close()
        print("Mock MQTT Server stopped")
    
    def handle_client(self, client_socket, client_addr):
        """处理客户端连接"""
        try:
            while True:
                # 读取MQTT固定头
                fixed_header = client_socket.recv(2)
                if not fixed_header:
                    break
                
                # 解析固定头
                msg_type = (fixed_header[0] >> 4) & 0x0F
                remaining_length = self.decode_remaining_length(client_socket, fixed_header[1])
                
                print(f"Received message type: {msg_type}, remaining length: {remaining_length}")
                
                # 根据消息类型处理
                if msg_type == MQTT_CONNECT:
                    self.handle_connect(client_socket, remaining_length)
                elif msg_type == MQTT_PUBLISH:
                    self.handle_publish(client_socket, remaining_length)
                else:
                    print(f"Unknown message type: {msg_type}")
                    # 读取剩余数据
                    client_socket.recv(remaining_length)
        except Exception as e:
            print(f"Error handling client {client_addr}: {e}")
        finally:
            print(f"Client disconnected: {client_addr}")
            client_socket.close()
    
    def decode_remaining_length(self, client_socket, first_byte):
        """解码MQTT剩余长度"""
        remaining_length = 0
        multiplier = 1
        
        byte = first_byte
        while True:
            remaining_length += (byte & 0x7F) * multiplier
            if (byte & 0x80) == 0:
                break
            multiplier *= 128
            byte = client_socket.recv(1)[0]
        
        return remaining_length
    
    def handle_connect(self, client_socket, remaining_length):
        """处理连接请求"""
        # 读取连接数据包
        conn_data = client_socket.recv(remaining_length)
        
        # 解析协议名
        proto_len = struct.unpack('!H', conn_data[0:2])[0]
        proto_name = conn_data[2:2+proto_len].decode()
        
        print(f"Received CONNECT request")
        print(f"  Protocol: {proto_name}")
        print(f"  Protocol level: {conn_data[2+proto_len]}")
        print(f"  Connect flags: {hex(conn_data[3+proto_len])}")
        
        # 发送连接确认
        connack_packet = bytearray()
        connack_packet.append((MQTT_CONNACK << 4) | 0x00)  # 固定头
        connack_packet.append(0x02)  # 剩余长度
        connack_packet.append(0x00)  # 保留位
        connack_packet.append(CONNACK_ACCEPTED)  # 连接返回码
        
        client_socket.send(connack_packet)
        print("Sent CONNACK packet (connection accepted)")
    
    def handle_publish(self, client_socket, remaining_length):
        """处理发布消息"""
        # 读取发布数据包
        publish_data = client_socket.recv(remaining_length)
        
        # 解析主题
        topic_len = struct.unpack('!H', publish_data[0:2])[0]
        topic = publish_data[2:2+topic_len].decode()
        
        # 解析消息内容
        message = publish_data[2+topic_len:].decode()
        
        print(f"Received PUBLISH message")
        print(f"  Topic: {topic}")
        print(f"  Message: {message}")
        print(f"  Message length: {len(message)}")

if __name__ == "__main__":
    server = MockMQTTServer()
    try:
        server.start()
    except KeyboardInterrupt:
        server.stop()
