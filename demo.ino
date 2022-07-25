#include <ESP8266WiFi.h>

const char *ssid     = "HUAWEI-47Q5RT1234";//这里写入网络的ssid
const char *password = "04546035489ma";//wifi密码
const char *host     = "192.168.3.3";//修改为Server服务端的IP，即你电脑的IP，确保在同一网络之下。

WiFiClient client;
const int tcpPort = 80;//修改为你建立的Server服务端的端口号，此端口号是创建服务器时指定的。

//接收状态机
uint8_t rStateMachine = 0; //状态位
uint8_t sumchkm = 0;      //接收和校验位
uint8_t xorchkm = 0;      //接收异或校验位
uint8_t m_DstAdr = 0x01;  //目的地址
uint8_t m_SrcAdr = 0x02;  //源地址
uint8_t lencnt = 0;  //接收长度计数器
uint8_t rcvcount = 0;  //接收长度
uint8_t m_ucData[16]; //接收缓冲区
uint8_t retval = 0;   //接收标志位



void setup() {
  //串口初始化
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  //Serial.println("hardware serial!");
  //连接wifi
  WiFi.begin(ssid, password);

  //在这里检测是否成功连接到目标网络，未连接则阻塞。
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  while (!client.connected())//若未连接到服务端，则客户端进行连接。
  {
    if (!client.connect(host, tcpPort))//实际上这一步就在连接服务端，如果连接上，该函数返回true
    {
      Serial.println("connection....");
      delay(500);

    }
  }

  //wifi->串口
  while (client.available())//available()表示是否可以获取到数据
  {
    //透传数据
    uint8_t c = client.read();
    Serial.write(c);
  }

  //串口->wifi
  if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
  {

    //接收未读数据
    size_t rCount = Serial.available();//串口缓冲器 返回的是缓冲区准确的可读字节数
    uint8_t rBuf[rCount];
    Serial.readBytes(rBuf, rCount); //从串口读取指定长度counti的字符到缓存数组sbuf。
    //client.write(rBuf, rCount);//wifi发送数据包
    //分字节处理数据
    uint8_t i = 0;
    for (i = 0; i < rCount; i++) {
      //
      uint8_t rData = rBuf[i];

      //解析数据帧
      switch (rStateMachine) {
        //帧头0 0x55
        case 0:
          if (rData == 0x55)     // 接收到帧头0
            rStateMachine = 1;
          else
            rStateMachine = 0;      // 状态机复位
          break;
        //帧头1 0xAA
        case 1:
          if (rData == 0xAA)     // 接收到帧头1
            rStateMachine = 2;
          else
            rStateMachine = 0;      // 状态机复位
          break;
        //帧头1 0x7E
        case 2:
          if (rData == 0x7E)      // 接收到帧头2
            rStateMachine = 3;
          else
            rStateMachine = 0;      // 状态机复位
          break;
        //目的地址
        case 3:
          sumchkm = rData;       // 开始计算累加、异或校验和
          xorchkm = rData;
          if (rData == m_DstAdr)     // 判断目的地址是否正确
            rStateMachine = 4;
          else
            rStateMachine = 0;
          break;
        //源地址
        case 4:
          sumchkm += rData;
          xorchkm ^= rData;
          if (rData == m_SrcAdr)     // 判断源地址是否正确
            rStateMachine = 5;
          else
            rStateMachine = 0;
          break;
        //数据长度
        case 5:
          lencnt = 0;            // 接收数据计数器
          rcvcount = rData;        // 接收数据长度
          sumchkm += rData;
          xorchkm ^= rData;
          rStateMachine = 6;
          break;
        //保存数据
        case 6:
          m_ucData[lencnt++] = rData;     // 数据保存
          sumchkm += rData;
          xorchkm ^= rData;
          if (lencnt == rcvcount)     // 判断数据是否接收完毕
            rStateMachine = 7;
          break;
        //累加和
        case 7:
          if (sumchkm == rData)     // 判断累加和是否相等
            rStateMachine = 8;
          else
            rStateMachine = 0;
          break;
        //异或校验和
        case 8:
          if (xorchkm == rData)     // 判断异或校验和是否相等
            rStateMachine = 9;
          else
            rStateMachine = 0;
          break;
        case 9:
          if (0x0D == rData)      // 判断是否接收到帧尾结束符
          {
            retval = 1;    // 置标志，表示一个数据包接收到
            //向服务端发送数据包
            uint8_t sBuf[rcvcount];
            uint8_t sCount = rcvcount;
            uint8_t n = 0;
            for (n = 0; n < rcvcount; n++)
            {
              sBuf[n] = m_ucData[n];
            }
            client.write(sBuf, sCount);
            retval = 0;
          }
          rStateMachine = 0;     // 复位状态机
          break;
        default:
          rStateMachine = 0;     // 复位状态机
          break;
      }
    }
  }
}
