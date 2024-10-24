#include "mqtt.h"
#include "cJSON.h"

char mqtt_post_msg[526];
uint32_t mqtt_tx_len;
const uint8_t g_packet_heart_reply[2] = {0xc0, 0x00};
unsigned char TxBuffer[512];
void * jx(char * RBuffer);
char * jshead(char * head,int sun);
// 用于线程接收信息的缓冲数组和下表计数值
static int RxCounter = 0;
static unsigned char RxBuffer[512];
extern void xingxi(int nero,int zi);

void * mqtt_recvdata(void * arg)
{
    int socket_fd = (int)arg;

    while(1) {

        memset(RxBuffer, 0, 512);
        RxCounter = read(socket_fd, RxBuffer, sizeof(RxBuffer));
        if(RxCounter == -1) {
            perror("read err");
            continue;
        }
        printf("字节数[%d]MQTT服务器发送的数据:", RxCounter);
        for(int i = 0; i < RxCounter; i++) {
            printf("%c", RxBuffer[i]);
        }
        printf("\n");
        jx(jshead(RxBuffer,512));
        sleep(1);
    }
}

//json头
char * jshead(char * head,int sun)
{
    for (size_t i = 0; i < sun; i++)
    {
        if (head[i] == '{')
        {
            return head + i;
        }
        
    }
    return NULL;
    
}

//解析json数据
void * jx(char * RBuffer)
{
    cJSON * root = cJSON_Parse(RBuffer);
    if(root == NULL) {
        printf("数据为空\n");
    } else {
        cJSON * params = cJSON_GetObjectItem(root, "params");
        if(params == NULL) {
            printf("params数据为空\n");
        } else {
            cJSON * power_switch = cJSON_GetObjectItem(params, "power_switch");
            if(power_switch == NULL) {
                printf("power_switch数据为空\n");
            } else {
                // 开关属性
                printf("power_switch数据:%d\n",power_switch->valueint);
                xingxi(1,power_switch->valueint);
            }
            cJSON * brightness = cJSON_GetObjectItem(params, "brightness");
            if(brightness == NULL) {
                printf("brightness数据为空\n");
            } else {
                // 亮度属性
                printf("brightness数据:%d\n",brightness->valueint);
                xingxi(2,brightness->valueint);
            }
            cJSON * rgb_color = cJSON_GetObjectItem(params, "rgb_color");
            printf("获取数据\n");
            if(rgb_color == NULL) {
                printf("rgb_color数据为空\n");
            } else {
                // 颜色属性
                cJSON * green = cJSON_GetObjectItem(rgb_color, "green");
                printf("绿色获取数据\n");
                if(green == NULL) {
                    printf("green数据为空\n");
                } else {
                    // 绿属性
                    printf("绿:%d\n",green->valueint);
                    xingxi(4,green->valueint);
                }
                cJSON * blue = cJSON_GetObjectItem(rgb_color, "blue");
                printf("获取蓝色数据\n");
                if(blue == NULL) {
                    printf("blue数据为空\n");
                } else {
                    // 蓝属性
                    printf("蓝:%d\n",blue->valueint);
                    xingxi(5,blue->valueint);
                }
                cJSON * red = cJSON_GetObjectItem(rgb_color, "red");
                printf("获取红色数据\n");
                if(red == NULL) {
                    printf("red数据为空\n");
                } else {
                    // 红属性
                    printf("红:%d\n",red->valueint);
                    xingxi(3,red->valueint);
                }
            }
        }
    }
}

/*
函数功能：根据传入的IP及端口号，使用TCP协议连接远程的TCP服务器  内部函数
参数1：ip地址  传入ipv4地址字符串即可 如："192.168.xxx.xxx"
参数2：port 端口号 16bit无符号
返回值： 客户端套接字描述符 成功
       -1 socket失败   -2 connect失败
*/
static int tcp_connect_server(char * ip, unsigned short port)
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1) {
        perror("socket fail");
        return -1;
    }
    printf("socket 成功 %d\n", socketfd);

    struct sockaddr_in addr;              // 专属结构体类型_in TCP/UDP        _un  本地通信
    addr.sin_family      = AF_INET;       // 协议族 IPv4
    addr.sin_addr.s_addr = inet_addr(ip); // 服务器主机的 IP指定
    addr.sin_port        = htons(port);   // 服务器进程的端口号PORT 16bit
    if(-1 == connect(socketfd, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect err");
        return -2;
    }
    printf("connect 成功 %d\n", socketfd);
    return socketfd;
}

// 配置MQTT连接MQTT服务器
int mqtt_connect_server(char * mqttserver_ip, unsigned short mqttserver_port)
{
    int ret = 0;
    int socket_fd;
    pthread_t tid;
    // 1、连接到目标MQTT-TCP服务器
    socket_fd = tcp_connect_server(mqttserver_ip, mqttserver_port);
    if(socket_fd < 0) {
        printf("连接MQTT服务器失败\n");
        return -1;
    }
    printf("连接MQTT TCP服务器成功\n");

    // 2、创建一个线程，专门用于接收MQTT服务器发送过来数据
    if(0 != pthread_create(&tid, NULL, mqtt_recvdata, (void *)socket_fd)) {
        perror("线程创建失败");
        return -2;
    }
    printf("用于接收服务器信息的线程创建成功\n");

    usleep(300000); // 等待一会

    // 当前MQTT客户端连接MQTT服务器  发送CONNECT报文
    if(mqtt_connect(socket_fd, MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD)) {
        printf("MQTT服务器连接失败\n");
        return -7;
    }
    printf("MQTT服务器连接成功\n");
    sleep(1);

    // MQTT订阅主题---这里是可选的  可以在主程序中自己决定订阅什么主题  以下是测试
    if(mqtt_subscribe_topic(socket_fd, MQTT_SUBSCRIBE_TOPIC, 0, 1)) {
        printf("MQTT订阅主题失败\n");
        return -8;
    }

    printf("MQTT订阅测试主题成功\n");

    return socket_fd;
}

// MQTT连接服务器的打包函数 提供发送的TCP客户端套接字  提供客户端ID  用户名  和 密码
// 如果连接的MQTT服务器不需要用户名和密码校验，则参数3和参数4填  "" 空字符串即可
int32_t mqtt_connect(int socket_fd, char * client_id, char * user_name, char * password)
{
    uint8_t encodedByte    = 0;
    uint32_t client_id_len = strlen(client_id);
    uint32_t user_name_len = strlen(user_name);
    uint32_t password_len  = strlen(password);
    uint32_t data_len;
    uint32_t cnt  = 2;
    uint32_t wait = 0;
    mqtt_tx_len   = 0;

    // 可变报头+Payload  每个字段包含两个字节的长度标识
    data_len = 10 + (client_id_len + 2) + (user_name_len + 2) + (password_len + 2);

    // 固定报头
    // 控制报文类型
    TxBuffer[mqtt_tx_len++] = 0x10; // MQTT Message Type CONNECT
    // 剩余长度(不包括固定头部)
    do {
        encodedByte = data_len % 128;
        data_len    = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if(data_len > 0) encodedByte = encodedByte | 128;
        TxBuffer[mqtt_tx_len++] = encodedByte;
    } while(data_len > 0);

    // 可变报头
    // 协议名
    TxBuffer[mqtt_tx_len++] = 0;   // Protocol Name Length MSB
    TxBuffer[mqtt_tx_len++] = 4;   // Protocol Name Length LSB
    TxBuffer[mqtt_tx_len++] = 'M'; // ASCII Code for M
    TxBuffer[mqtt_tx_len++] = 'Q'; // ASCII Code for Q
    TxBuffer[mqtt_tx_len++] = 'T'; // ASCII Code for T
    TxBuffer[mqtt_tx_len++] = 'T'; // ASCII Code for T
    // 协议级别
    TxBuffer[mqtt_tx_len++] = 4; // MQTT Protocol version = 4
    // 连接标志
    TxBuffer[mqtt_tx_len++] = 0xc2; // conn flags
    TxBuffer[mqtt_tx_len++] = 0;    // Keep-alive Time Length MSB
    TxBuffer[mqtt_tx_len++] = 60;   // Keep-alive Time Length LSB  60S心跳包

    TxBuffer[mqtt_tx_len++] = BYTE1(client_id_len); // Client ID length MSB
    TxBuffer[mqtt_tx_len++] = BYTE0(client_id_len); // Client ID length LSB
    memcpy((void *)&TxBuffer[mqtt_tx_len], client_id, client_id_len);
    mqtt_tx_len += client_id_len;

    if(user_name_len > 0) {
        TxBuffer[mqtt_tx_len++] = BYTE1(user_name_len); // user_name length MSB
        TxBuffer[mqtt_tx_len++] = BYTE0(user_name_len); // user_name length LSB
        memcpy((void *)&TxBuffer[mqtt_tx_len], user_name, user_name_len);
        mqtt_tx_len += user_name_len;
    }

    if(password_len > 0) {
        TxBuffer[mqtt_tx_len++] = BYTE1(password_len); // password length MSB
        TxBuffer[mqtt_tx_len++] = BYTE0(password_len); // password length LSB
        memcpy((void *)&TxBuffer[mqtt_tx_len], password, password_len);
        mqtt_tx_len += password_len;
    }

    while(cnt--) {
        memset((void *)RxBuffer, 0, sizeof(RxBuffer));
        RxCounter = 0;
        // mqtt_send_bytes((void *)TxBuffer,mqtt_tx_len);
        // write发送CONNECT协议包
        write(socket_fd, TxBuffer, mqtt_tx_len);

        // 等待3s时间
        wait = 3000;
        while(wait--) {
            usleep(1000); // 1ms
            // 检查连接确认固定报头
            if((RxBuffer[0] == 0x20) && (RxBuffer[1] == 0x02)) {
                printf("连接报文服务器发回报文字节数:%d\n", RxCounter);
                if(RxBuffer[3] == 0x00) {
                    printf("连接已被服务器端接受，连接确认成功\n");
                    // 连接成功
                    return 0;
                } else {
                    switch(RxBuffer[3]) {
                        case 1: printf("连接已拒绝，不支持的协议版本\n"); break;
                        case 2: printf("连接已拒绝，不合格的客户端标识符\n"); break;
                        case 3: printf("连接已拒绝，服务端不可用\n"); break;
                        case 4: printf("连接已拒绝，无效的用户或密码\n"); break;
                        case 5: printf("连接已拒绝，未授权\n"); break;
                        default: printf("未知响应\n"); break;
                    }
                    return 0;
                }
            }
        }
    }

    return -1;
}

/**
 * @brief  MQTT订阅/取消订阅数据打包函数
 * @param  topic  		主题
 * @param  qos    		消息等级
 * @param  whether: 	1订阅 / 0取消订阅请求包
 * @retval 0：成功；
 * 		1：失败；
 */
int32_t mqtt_subscribe_topic(int socket_fd, char * topic, uint8_t qos, uint8_t whether)
{
    uint8_t encodedByte = 0;
    uint32_t cnt        = 2;
    uint32_t wait       = 0;

    uint32_t topiclen = strlen(topic);
    uint32_t data_len = 2 + (topiclen + 2) + (whether ? 1 : 0); // 可变报头的长度（2字节）加上有效载荷的长度

    mqtt_tx_len = 0;

    // 固定报头
    // 控制报文类型
    if(whether)
        TxBuffer[mqtt_tx_len++] = 0x82; // 消息类型和标志订阅
    else
        TxBuffer[mqtt_tx_len++] = 0xA2; // 取消订阅

    // 剩余长度
    do {
        encodedByte = data_len % 128;
        data_len    = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if(data_len > 0) encodedByte = encodedByte | 128;
        TxBuffer[mqtt_tx_len++] = encodedByte;
    } while(data_len > 0);

    // 可变报头
    TxBuffer[mqtt_tx_len++] = 0;    // 消息标识符 MSB
    TxBuffer[mqtt_tx_len++] = 0x01; // 消息标识符 LSB

    // 有效载荷
    TxBuffer[mqtt_tx_len++] = BYTE1(topiclen); // 主题长度 MSB
    TxBuffer[mqtt_tx_len++] = BYTE0(topiclen); // 主题长度 LSB
    memcpy((void *)&TxBuffer[mqtt_tx_len], topic, topiclen);

    mqtt_tx_len += topiclen;

    if(whether) {
        TxBuffer[mqtt_tx_len++] = qos; // QoS级别
    }

    while(cnt--) {
        RxCounter = 0;
        memset((void *)RxBuffer, 0, sizeof(RxBuffer));
        // mqtt_send_bytes((void *)TxBuffer,mqtt_tx_len);
        write(socket_fd, TxBuffer, mqtt_tx_len);

        wait = 3000; // 等待3s时间
        while(wait--) {
            usleep(1000);

            // 检查订阅确认报头
            if(RxBuffer[0] == 0x90) {
                printf("订阅主题确认成功\n");

                // 获取剩余长度
                if(RxBuffer[1] == 3) {
                    printf("Success - Maximum QoS 0 is %02X\n", RxBuffer[2]);
                    printf("Success - Maximum QoS 2 is %02X\n", RxBuffer[3]);
                    printf("Failure is %02X\n", RxBuffer[4]);
                }
                // 获取剩余长度
                if(RxBuffer[1] == 2) {
                    printf("Success - Maximum QoS 0 is %02X\n", RxBuffer[2]);
                    printf("Success - Maximum QoS 2 is %02X\n", RxBuffer[3]);
                }

                // 获取剩余长度
                if(RxBuffer[1] == 1) {
                    printf("Success - Maximum QoS 0 is %02X\n", RxBuffer[2]);
                }

                // 订阅成功
                return 0;
            }
        }
    }

    if(cnt) return 0; // 订阅成功

    return -1;
}

/**
 * @brief  MQTT发布消息 数据打包函数
 * @param  topic  		主题
 * @param  message  	消息
 * @param  qos    		消息等级
 * @retval 0：成功；
 * 		1：失败；
 */
uint32_t mqtt_publish_data(int socket_fd, char * topic, char * message, uint8_t qos)
{
    // static
    uint16_t id            = 0;
    uint32_t topicLength   = strlen(topic);
    uint32_t messageLength = strlen(message);

    uint32_t data_len;
    uint8_t encodedByte;

    mqtt_tx_len = 0;
    // 有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
    // QOS为0时没有标识符
    // 数据长度             主题名   报文标识符   有效载荷
    if(qos)
        data_len = (2 + topicLength) + 2 + messageLength;
    else
        data_len = (2 + topicLength) + messageLength;

    // 固定报头
    // 控制报文类型
    TxBuffer[mqtt_tx_len++] = 0x30; // MQTT Message Type PUBLISH

    // 剩余长度
    do {
        encodedByte = data_len % 128;
        data_len    = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if(data_len > 0) encodedByte = encodedByte | 128;
        TxBuffer[mqtt_tx_len++] = encodedByte;
    } while(data_len > 0);

    TxBuffer[mqtt_tx_len++] = BYTE1(topicLength); // 主题长度MSB
    TxBuffer[mqtt_tx_len++] = BYTE0(topicLength); // 主题长度LSB

    memcpy((char *)&TxBuffer[mqtt_tx_len], topic, topicLength); // 拷贝主题

    mqtt_tx_len += topicLength;

    // 报文标识符
    if(qos) {
        TxBuffer[mqtt_tx_len++] = BYTE1(id);
        TxBuffer[mqtt_tx_len++] = BYTE0(id);
        id++;
    }

    memcpy((char *)&TxBuffer[mqtt_tx_len], message, messageLength);

    mqtt_tx_len += messageLength;

    // mqtt_send_bytes((uint8_t *)TxBuffer,mqtt_tx_len);
    //write(socket_fd, (void *)TxBuffer, mqtt_tx_len);
    send (socket_fd, (void *)TxBuffer, mqtt_tx_len,0);

    // Qos等级设置的是00，因此阿里云物联网平台是没有返回响应信息的;
    return mqtt_tx_len;
}

// 发送心跳包
int32_t mqtt_send_heart(int socket_fd)
{
    uint8_t buf[2] = {0xC0, 0x00};
    uint32_t cnt   = 2;
    uint32_t wait  = 0;

    while(cnt--) {
        // mqtt_send_bytes(buf,2);
        write(socket_fd, buf, 2);
        memset((void *)RxBuffer, 0, sizeof(RxBuffer));
        RxCounter = 0;

        wait = 3000; // 等待3s时间

        while(wait--) {
            usleep(1000);

            // 检查心跳响应固定报头
            if((RxBuffer[0] == 0xD0) && (RxBuffer[1] == 0x00)) {
                printf("心跳响应确认成功，服务器在线。\n");
                return 0;
            }
        }
    }
    printf("心跳响应确认失败，服务器离线\n");
    return -1;
}

// MQTT无条件断开
void mqtt_disconnect(int socket_fd)
{
    uint8_t buf[2] = {0xe0, 0x00};

    // mqtt_send_bytes(buf,2);
    write(socket_fd, buf, 2);

    // esp8266_disconnect_server();
    close(socket_fd);
}
