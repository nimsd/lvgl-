#ifndef __MQTT_H
#define __MQTT_H

//包含头文件
#include <stdio.h>
#include <string.h>
#include <sys/types.h>      
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

//宏定义
//此处是阿里云服务器的公共实例登陆配置------------注意修改为自己的云服务设备信息！！！！
#define MQTT_CLIENTID			"MRJO085OTLRGB_1"
#define MQTT_USARNAME			"MRJO085OTLRGB_1;21010406;83Io9;92233720368547758"
#define MQTT_PASSWD				"fc9d81b4a1c66027e23ff39c3ed529608403ab4c;hmacsha1"
#define	MQTT_PUBLISH_TOPIC		"$thing/up/property/MRJO085OTL/RGB_1"
#define MQTT_SUBSCRIBE_TOPIC	"$thing/down/property/MRJO085OTL/RGB_1"  //本地MQTT服务器测试的订阅主题  用的MQTT.fx 或者emqx软件测试

/*pass=F7bhffL9unP2kXEjzQKaDQ==*/

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
	
#define CONNECT_MQTT_LED(x)	PBout(3)=(x)?0:1
//变量声明

//函数声明
int mqtt_connect_server(char *mqttserver_ip, unsigned short mqttserver_port);//配置MQTT服务器
int32_t mqtt_connect(int socket_fd,char *client_id,char *user_name,char *password);	//MQTT连接服务器
int32_t mqtt_subscribe_topic(int socket_fd,char *topic,uint8_t qos,uint8_t whether);	//MQTT消息订阅
uint32_t mqtt_publish_data(int socket_fd,char *topic, char *message, uint8_t qos);	//MQTT消息发布
int32_t mqtt_send_heart(int socket_fd);											//MQTT发送心跳包
void mqtt_disconnect(int socket_fd);												//MQTT无条件断开
#endif
