#include "Int_BLE.h"

// 单次接受返回信息
#define ESP32_RCE_BUFF_LEN 128
uint8_t esp32_recv_buff[ESP32_RCE_BUFF_LEN];
uint16_t esp32_recv_len = 0;

// 拼接单次接受的返回信息
#define ESP32_FULL_BUFF_LEN 256
uint8_t esp32_full_buff[ESP32_FULL_BUFF_LEN];
uint16_t esp32_full_len = 0;

void Int_BLE_SendCmd(uint8_t *cmd)
{
    HAL_UART_Transmit(&huart2, cmd, strlen((char *)cmd), 1000);

    // 处理返回信息
    // HAL_UART_Receive => 一直接受数据直到缓冲区满或者超时
    // 每次接受到的消息,一直接受到包含OK或者ERROR为止
    //      从哪里开始清除  0=清除  要清除多少个字节
    memset(esp32_full_buff, 0, ESP32_FULL_BUFF_LEN);
    // 对上次接受的消息进行清空
    esp32_full_len = 0;
    // do while 先执行一次代码,在依旧条件判断是否继续执行
    do
    {
        // 每次接受信息数据之前清空使用的缓冲区
        memset(esp32_recv_buff, 0, ESP32_RCE_BUFF_LEN);
        esp32_recv_len = 0;
        // HAL_UARTEx_ReceiveToIdle => 接收到空闲帧即停止
        HAL_UARTEx_ReceiveToIdle(&huart2, esp32_recv_buff, ESP32_RCE_BUFF_LEN,
                                 &esp32_recv_len, 1000);
        // 每次接受的消息进行拼接
        memcpy(&esp32_full_buff[esp32_full_len], esp32_recv_buff, esp32_recv_len);
        esp32_full_len += esp32_recv_len;

    } while (strstr((char *)esp32_full_buff, "ok") == NULL &&
             strstr((char *)esp32_full_buff, "ERROR") == NULL);

    printf("%d:%s\r\n", esp32_full_len, esp32_full_buff);
}

void Int_BLE_Init(void)
{
    // 1. 初始化底层驱动
    //  MX_USART2_UART_Init();

    // 2. 重启ESP32
    Int_BLE_SendCmd("AT+RST=0\r\n");
    // 芯片设计缺陷的原因复位以后需要加一个延时 实验出来的 不是官方文档的要求
    HAL_Delay(3000);

    // 使用AT指令发送ESP命令的时候 不能连续发送命令一定要等待上一条命令执行完成才能发下一条
    // 3.查询版本信息
    Int_BLE_SendCmd("AT+GMR\r\n");


 
    // 4.0 配置蓝牙
    // 4.1 配置蓝牙角色为服务端
    Int_BLE_SendCmd("AT+BLEINIT=2\r\n");

    // 4.2 创建服务
    Int_BLE_SendCmd("AT+BLEGATTSSRVCRE\r\n");

    // 4.3 开启服务
    Int_BLE_SendCmd("AT+BLEGATTSSRVSTART\r\n");

    // 4.4 设置广播参数
    Int_BLE_SendCmd("AT+BLEADVPARAM=50,50,0,0,7,0,,\r\n");

    // 4.5 设置广播数据
    Int_BLE_SendCmd("AT+BLEADVDATAEX\"ESP32_C3-test\",\"A002\",\"e102030405\",1\r\n");

    // 4.5 开始广播
    Int_BLE_SendCmd("AT+BLEADVSTART\r\n");
}
