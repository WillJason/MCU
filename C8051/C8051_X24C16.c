/*****************************************************************************
文件名称：24C16.C
作    者：zp
版    本：V0.1
说    明：C8051F410读写X24C16测试
修改记录：2010-06-21
******************************************************************************/
#include<c8051F410.h>


#define  SYSCLK         24500000       // 系统时钟
#define  SMB_FREQUENCY  50000          // SCL时钟


#define  WRITE          0x00           //SMBus“写”命令
#define  READ           0x01           //SMBus“读”命令
#define  EEPROM_ADDR    0xA0           //SMBus从设备器件地址
#define  SMB_BUFF_SIZE  0x10           //单次传输允许最大传输字节数量


/******************************SMBus状态矢量************************************/
#define  SMB_MTSTA      0xE0           //主发送：启动传输          
#define  SMB_MTDB       0xC0           //主发送：数据字节发送        
#define  SMB_MRDB       0x80           //主接收：数据字节接收


/******************************全局变量***********************************/
unsigned char *pSMB_DATA_IN;           //所有接收到的数据写到这里
unsigned char SMB_SINGLEBYTE_OUT;      //单个字节写
unsigned char *pSMB_DATA_OUT;          //所有要发送的数据从这里读取
unsigned char SMB_DATA_LEN;            //当前需要发送或接收的数据数量
unsigned int  WORD_ADDR;               //从设备内部地址
unsigned char TARGET;                  //从设备目标地址
bit SMB_BUSY = 0;                      //传输忙标志：正在写入或者读取的时候SMB_BUSY = 1;
bit SMB_RW;                            //当前总线传输方向选择标志
bit SMB_SENDWORDADDR;                  //此标志位置位，则允许在发送完从设备地址后，发送器件内部地址
bit SMB_RANDOMREAD;                    //此标志位置位，则在发送完器件内部地址后，重新发送一个启动位
bit SMB_ACKPOLL;                       //此标志位置位，则一直发送重启信号，直到从设备发起一个确认其地址信号


/******************************16位MCU内部寄存器声明************************/
sfr16    TMR3RL   = 0x92;    //T3重装载寄存器
sfr16    TMR3     = 0x94;    //T3计数寄存器


sbit SDA = P0^0;
sbit SCL = P0^1;
sbit WP  = P2^5;


/*************************************************************************/
/*函数名称：SYSCLK_Init 
/*函数功能：系统时钟初始化
/*入口参数：无 
/*出口参数：无 
/*备 注： 
/*************************************************************************/ 
void SYSCLK_Init(void)
{  
   OSCICN = 0x87;                     //24.5MHz     
   RSTSRC = 0x04;                     //使能丢失时钟监测器
}
/*************************************************************************/
/*函数名称：SMBus_Init
/*函数功能：SMBus初始化
/*入口参数：无 
/*出口参数：无
/*备 注：  
/*************************************************************************/ 
void SMBus_Init(void)
{
   SMB0CF = 0x5D;                      //SMBus从方式禁止
                                       //允许SDA建立和保持时间扩展
            //允许SMBus SCL超时检测
            //允许 SMBus空闭超时检测
            //时钟源：定时器1溢出                              
   SMB0CF |= 0x80;                     //使能SMBus
}
/*************************************************************************/
/*函数名称：Timer1_Init 
/*函数功能：T1用来选择SMBus的传输速率
/*入口参数：无 
/*出口参数：无 
/*备 注： 
/*************************************************************************/ 
void Timer1_Init (void)
{


#if ((SYSCLK/SMB_FREQUENCY/3) < 255)
   #define SCALE 1
      CKCON |= 0x08;                   //T1系统时钟
#elif ((SYSCLK/SMB_FREQUENCY/4/3) < 255)
   #define SCALE 4
      CKCON |= 0x01;
      CKCON &= ~0x0A;                  //T1四分频时钟
#endif


   TMOD = 0x20;                        //T1－8位重装载模式


   TH1 = -(SYSCLK/SMB_FREQUENCY/12/3); //位速率
                                      
   TL1 = TH1;                          
   TR1 = 1;                            //启动T1
}
/*************************************************************************/
/*函数名称：Timer3_Init 
/*函数功能：T3用来监测SCL低电平超时
/*入口参数：无 
/*出口参数：无 
/*备 注： 
/*************************************************************************/ 
void Timer3_Init(void)
{
   TMR3CN = 0x00;                      //T3配置为16位自动重装载模式,禁止低位中断
   CKCON &= ~0x40;                     // T3使用系统时钟12分频
   TMR3RL = -(SYSCLK/12/40);           
   TMR3 = TMR3RL;                      //T3配置为超过25ms后溢出,用作SMBus低电平超时检测


   EIE1 |= 0x80;                       // 使能T3中断
   TMR3CN |= 0x04;                     // 启动T3
}
/*************************************************************************/
/*函数名称：PORT_Init 
/*函数功能：测试端口初始化 
/*入口参数：无
/*出口参数：无 
/*备 注： 
/*************************************************************************/ 
void PORT_Init(void)
{
   P0MDOUT = 0x00;                  //P0口漏极开路    


   P1MDOUT |= 0xff;                 //LED接口推挽输出  


   XBR0 = 0x04;                     //SMBus引脚    
   XBR1 = 0x40;                     //使能交叉开关   
}
/*************************************************************************/
/*函数名称：Device_Init 
/*函数功能：系统设备初始化 
/*入口参数：无 
/*出口参数：无 
/*备 注： 
/*************************************************************************/ 
void Device_Init()
{
  SYSCLK_Init();             //系统时钟初始化
  PORT_Init();               //端口初始化
  Timer3_Init();             //定时器3初始化
  Timer1_Init();             //定时器1初始化
  SMBus_Init();              //SMBus初始化
} 
/*************************************************************************/
/*函数名称：EEPROM_ByteWrite 
/*函数功能：往地址addr写入数据dat
/*入口参数：addr：内部地址0～7FF,其中高三位为24C16分区地址选择;
             dat：需要写入的数据0～255
/*出口参数：无 
/*备 注：   
/*************************************************************************/ 
void EEPROM_ByteWrite(unsigned int addr,unsigned char dat)
{
  while(SMB_BUSY);
  SMB_BUSY = 1;
  TARGET = EEPROM_ADDR;
  SMB_RW = WRITE;
  SMB_SENDWORDADDR = 1;
  SMB_RANDOMREAD = 0;
  SMB_ACKPOLL = 1;
  WORD_ADDR = addr;
  SMB_SINGLEBYTE_OUT = dat;
  pSMB_DATA_OUT = &SMB_SINGLEBYTE_OUT;
  SMB_DATA_LEN = 1;
  STA = 1;
}
/*************************************************************************/
/*函数名称：EEPROM_WriteArray
/*函数功能：向从设备起始址dest_addr开始写入长度为len的一连串数据src_addr 
/*入口参数：dest_addr:内部起始地址;src_addr:一连串数据的地址;len:数据量
/*出口参数：无 
/*备 注： 
/*************************************************************************/ 
void EEPROM_WriteArray(unsigned int dest_addr, unsigned char* src_addr,
                       unsigned char len)
{
   unsigned char i;
   unsigned char* pData = (unsigned char*) src_addr;


   for( i = 0; i < len; i++ ){
      EEPROM_ByteWrite(dest_addr++, *pData++);
   }
}
/*************************************************************************/
/*函数名称：EEPROM_ByteRead 
/*函数功能：从地址addr读取数据 
/*入口参数：addr 
/*出口参数：retval：读到的数据 
/*备 注： 
/*************************************************************************/  
unsigned char EEPROM_ByteRead(unsigned int addr)
{
  unsigned char retval;
  while(SMB_BUSY);
  SMB_BUSY = 1;
  TARGET = EEPROM_ADDR;
  SMB_RW = WRITE;
  SMB_SENDWORDADDR = 1;
  SMB_RANDOMREAD = 1;
  SMB_ACKPOLL = 1;
  WORD_ADDR = addr;
  pSMB_DATA_IN = &retval;
  SMB_DATA_LEN = 1;
  STA = 1;
  while(SMB_BUSY);
  return retval;
}
/*************************************************************************/
/*函数名称：EEPROM_ReadArray 
/*函数功能：从地址src_addr开始读取数量为len的数据。装到dest_addr里 
/*入口参数：*dest_addr,src_addr,len
/*出口参数： 
/*备 注： 
/*************************************************************************/ 
void EEPROM_ReadArray(unsigned char* dest_addr, unsigned int src_addr,
                       unsigned char len)
{
  while (SMB_BUSY);
  SMB_BUSY = 1;
  TARGET = EEPROM_ADDR;
  SMB_RW = WRITE;
  SMB_SENDWORDADDR = 1;
  SMB_RANDOMREAD = 1;
  SMB_ACKPOLL = 1;
  WORD_ADDR = src_addr;
  pSMB_DATA_IN = (unsigned char*) dest_addr;
  SMB_DATA_LEN = len;
  STA = 1;
  while(SMB_BUSY);
}
/*************************************************************************/
/*函数名称：Timer3_ISR 
/*函数功能：用于SCL低电平超时检测,重启SMBus 
/*入口参数：无
/*出口参数：无
/*备 注： 
/*************************************************************************/ 
void Timer3_ISR (void) interrupt 14
{
   SMB0CF &= ~0x80;                    // 禁止SMBus
   SMB0CF |= 0x80;                     // 重启SMBus
   TMR3CN &= ~0x80;                   
   SMB_BUSY = 0;                       
}
/*************************************************************************/
/*函数名称：SMBus_ISR 
/*函数功能：用于检测SMBus的工作状态,并进行相应的数据处理
/*入口参数：无 
/*出口参数：无 
/*备 注： 
/*************************************************************************/ 
void SMBus_ISR(void) interrupt 7
{
   bit FAIL = 0;                       //传输失败标志
   unsigned char temp;
   static char i;                      //发送或接收数据数量


   static bit SEND_START = 0;          //启动标志位


   switch (SMB0CN & 0xF0)              // 状态变量
   {
      //主发送或接收: 启动条件已发送
      case SMB_MTSTA:
         temp = (unsigned char)(WORD_ADDR>>8);
         temp &= 0x07;
   temp <<=1;                    //取出地址的8~10位,合并到器件选择命令字节中
   SMB0DAT = TARGET|SMB_RW|temp; //装载目标地址
         STA = 0;                      //手动清除启动位
         i = 0;                        //重置数据位计数器
         break;


      //主发送: 数据位或从地址已发送
      case SMB_MTDB:
         if(ACK)                       // 从地址或数据位应答？
         {                            
            if (SEND_START)
            {
               STA = 1;
               SEND_START = 0;
               break;
            }
            if(SMB_SENDWORDADDR)       //是否发送器件内部地址?
            {
               SMB_SENDWORDADDR = 0;   //清标志位
               SMB0DAT =(unsigned char)WORD_ADDR;    //发送低8位器件内部地址


               if (SMB_RANDOMREAD)
               {
                  SEND_START = 1;     
                  SMB_RW = READ;
               }


               break;
            }


            if (SMB_RW==WRITE)         //如果此次发送是写操作?
            {


               if (i < SMB_DATA_LEN)   //数据发完?
               {
                  //发数据
                  SMB0DAT = *pSMB_DATA_OUT;


                  //发送的数据地址增加，连续写入数据
                  pSMB_DATA_OUT++;


                  //数据数量计数器加一
                  i++;
               }
               else
               {
                 STO = 1;              //置位STO以结束此次传输
                 SMB_BUSY = 0;         //清除忙标志位
               }
            }
            else {}                    //如果此次是个读操作,不表示关心,
                                       
         }
         else                          //如果收到NACK,
         {
            if(SMB_ACKPOLL)
            {
               STA = 1;                //重启发送
            }
            else
            {
               FAIL = 1;              
            }                         
         }
         break;


      //主接收: 字节接收
      case SMB_MRDB:
         if ( i < SMB_DATA_LEN )       //如果没接收完?
         {
            *pSMB_DATA_IN = SMB0DAT;   
            pSMB_DATA_IN++;           
            i++;                       
            ACK = 1;
         }
         if (i == SMB_DATA_LEN)        
         {
            SMB_BUSY = 0;             
            ACK = 0;                  
                                     
            STO = 1;                  
         }
         break;
      default:
         FAIL = 1;                
         break;
   }
   if (FAIL)                           //如果传输失败
   {
      SMB0CF &= ~0x80;                
      SMB0CF |= 0x80;
      STA = 0;
      STO = 0;
      ACK = 0;
      SMB_BUSY = 0;                    //释放SMBus
      FAIL = 0;
   }
   SI = 0;                             //清除中断标志位
}
/*************************************************************************/
/*函数名称：main 
/*函数功能：主函数 
/*入口参数：无 
/*出口参数：无 
/*备 注： 
/*************************************************************************/ 
void main()
{
  unsigned char temp_char=0;
  bit error_flag = 0;
  char in_buff[8] = {0};             //输入数据寄存器
  char out_buff[8] = "ABCDEFG";      //输出数据寄存器
  unsigned char i;
  PCA0MD &= ~0x40;                   //关闭看门狗
  Device_Init();                     //设备初始化
  WP = 0;
  EIE1 |= 0x01;                      //允许SMBus中断
  EA = 1;                            //开启总中断
  P1 = 0X00; 
  EEPROM_ByteWrite(0x7ff, 0xaa);
  temp_char = EEPROM_ByteRead(0x7ff);
  if(temp_char!=0xAA)
  {
    error_flag = 1;
  }
  EEPROM_ByteWrite(0x025, 0xBB);
  EEPROM_ByteWrite(0x038, 0xCC);
  temp_char = EEPROM_ByteRead(0x025);
  if (temp_char != 0xBB)
   {
      error_flag = 1;
   }
  temp_char = EEPROM_ByteRead(0x038);
  if (temp_char != 0xCC)
   {
      error_flag = 1;
   }
  EEPROM_WriteArray(0x150, out_buff, sizeof(out_buff));
  EEPROM_ReadArray(in_buff, 0x150, sizeof(in_buff));
  for (i = 0; i < sizeof(in_buff); i++)
   {
      if (in_buff[i] != out_buff[i])
      {
         error_flag = 1;
      }
   }
  if (error_flag == 0)
   {    
      P1 = 0XFF;
   }
  while(1);
}

