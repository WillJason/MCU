/*****************************************************************************
�ļ����ƣ�24C16.C
��    �ߣ�zp
��    ����V0.1
˵    ����C8051F410��дX24C16����
�޸ļ�¼��2010-06-21
******************************************************************************/
#include<c8051F410.h>


#define  SYSCLK         24500000       // ϵͳʱ��
#define  SMB_FREQUENCY  50000          // SCLʱ��


#define  WRITE          0x00           //SMBus��д������
#define  READ           0x01           //SMBus����������
#define  EEPROM_ADDR    0xA0           //SMBus���豸������ַ
#define  SMB_BUFF_SIZE  0x10           //���δ�������������ֽ�����


/******************************SMBus״̬ʸ��************************************/
#define  SMB_MTSTA      0xE0           //�����ͣ���������          
#define  SMB_MTDB       0xC0           //�����ͣ������ֽڷ���        
#define  SMB_MRDB       0x80           //�����գ������ֽڽ���


/******************************ȫ�ֱ���***********************************/
unsigned char *pSMB_DATA_IN;           //���н��յ�������д������
unsigned char SMB_SINGLEBYTE_OUT;      //�����ֽ�д
unsigned char *pSMB_DATA_OUT;          //����Ҫ���͵����ݴ������ȡ
unsigned char SMB_DATA_LEN;            //��ǰ��Ҫ���ͻ���յ���������
unsigned int  WORD_ADDR;               //���豸�ڲ���ַ
unsigned char TARGET;                  //���豸Ŀ���ַ
bit SMB_BUSY = 0;                      //����æ��־������д����߶�ȡ��ʱ��SMB_BUSY = 1;
bit SMB_RW;                            //��ǰ���ߴ��䷽��ѡ���־
bit SMB_SENDWORDADDR;                  //�˱�־λ��λ���������ڷ�������豸��ַ�󣬷��������ڲ���ַ
bit SMB_RANDOMREAD;                    //�˱�־λ��λ�����ڷ����������ڲ���ַ�����·���һ������λ
bit SMB_ACKPOLL;                       //�˱�־λ��λ����һֱ���������źţ�ֱ�����豸����һ��ȷ�����ַ�ź�


/******************************16λMCU�ڲ��Ĵ�������************************/
sfr16    TMR3RL   = 0x92;    //T3��װ�ؼĴ���
sfr16    TMR3     = 0x94;    //T3�����Ĵ���


sbit SDA = P0^0;
sbit SCL = P0^1;
sbit WP  = P2^5;


/*************************************************************************/
/*�������ƣ�SYSCLK_Init 
/*�������ܣ�ϵͳʱ�ӳ�ʼ��
/*��ڲ������� 
/*���ڲ������� 
/*�� ע�� 
/*************************************************************************/ 
void SYSCLK_Init(void)
{  
   OSCICN = 0x87;                     //24.5MHz     
   RSTSRC = 0x04;                     //ʹ�ܶ�ʧʱ�Ӽ����
}
/*************************************************************************/
/*�������ƣ�SMBus_Init
/*�������ܣ�SMBus��ʼ��
/*��ڲ������� 
/*���ڲ�������
/*�� ע��  
/*************************************************************************/ 
void SMBus_Init(void)
{
   SMB0CF = 0x5D;                      //SMBus�ӷ�ʽ��ֹ
                                       //����SDA�����ͱ���ʱ����չ
            //����SMBus SCL��ʱ���
            //���� SMBus�ձճ�ʱ���
            //ʱ��Դ����ʱ��1���                              
   SMB0CF |= 0x80;                     //ʹ��SMBus
}
/*************************************************************************/
/*�������ƣ�Timer1_Init 
/*�������ܣ�T1����ѡ��SMBus�Ĵ�������
/*��ڲ������� 
/*���ڲ������� 
/*�� ע�� 
/*************************************************************************/ 
void Timer1_Init (void)
{


#if ((SYSCLK/SMB_FREQUENCY/3) < 255)
   #define SCALE 1
      CKCON |= 0x08;                   //T1ϵͳʱ��
#elif ((SYSCLK/SMB_FREQUENCY/4/3) < 255)
   #define SCALE 4
      CKCON |= 0x01;
      CKCON &= ~0x0A;                  //T1�ķ�Ƶʱ��
#endif


   TMOD = 0x20;                        //T1��8λ��װ��ģʽ


   TH1 = -(SYSCLK/SMB_FREQUENCY/12/3); //λ����
                                      
   TL1 = TH1;                          
   TR1 = 1;                            //����T1
}
/*************************************************************************/
/*�������ƣ�Timer3_Init 
/*�������ܣ�T3�������SCL�͵�ƽ��ʱ
/*��ڲ������� 
/*���ڲ������� 
/*�� ע�� 
/*************************************************************************/ 
void Timer3_Init(void)
{
   TMR3CN = 0x00;                      //T3����Ϊ16λ�Զ���װ��ģʽ,��ֹ��λ�ж�
   CKCON &= ~0x40;                     // T3ʹ��ϵͳʱ��12��Ƶ
   TMR3RL = -(SYSCLK/12/40);           
   TMR3 = TMR3RL;                      //T3����Ϊ����25ms�����,����SMBus�͵�ƽ��ʱ���


   EIE1 |= 0x80;                       // ʹ��T3�ж�
   TMR3CN |= 0x04;                     // ����T3
}
/*************************************************************************/
/*�������ƣ�PORT_Init 
/*�������ܣ����Զ˿ڳ�ʼ�� 
/*��ڲ�������
/*���ڲ������� 
/*�� ע�� 
/*************************************************************************/ 
void PORT_Init(void)
{
   P0MDOUT = 0x00;                  //P0��©����·    


   P1MDOUT |= 0xff;                 //LED�ӿ��������  


   XBR0 = 0x04;                     //SMBus����    
   XBR1 = 0x40;                     //ʹ�ܽ��濪��   
}
/*************************************************************************/
/*�������ƣ�Device_Init 
/*�������ܣ�ϵͳ�豸��ʼ�� 
/*��ڲ������� 
/*���ڲ������� 
/*�� ע�� 
/*************************************************************************/ 
void Device_Init()
{
  SYSCLK_Init();             //ϵͳʱ�ӳ�ʼ��
  PORT_Init();               //�˿ڳ�ʼ��
  Timer3_Init();             //��ʱ��3��ʼ��
  Timer1_Init();             //��ʱ��1��ʼ��
  SMBus_Init();              //SMBus��ʼ��
} 
/*************************************************************************/
/*�������ƣ�EEPROM_ByteWrite 
/*�������ܣ�����ַaddrд������dat
/*��ڲ�����addr���ڲ���ַ0��7FF,���и���λΪ24C16������ַѡ��;
             dat����Ҫд�������0��255
/*���ڲ������� 
/*�� ע��   
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
/*�������ƣ�EEPROM_WriteArray
/*�������ܣ�����豸��ʼַdest_addr��ʼд�볤��Ϊlen��һ��������src_addr 
/*��ڲ�����dest_addr:�ڲ���ʼ��ַ;src_addr:һ�������ݵĵ�ַ;len:������
/*���ڲ������� 
/*�� ע�� 
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
/*�������ƣ�EEPROM_ByteRead 
/*�������ܣ��ӵ�ַaddr��ȡ���� 
/*��ڲ�����addr 
/*���ڲ�����retval������������ 
/*�� ע�� 
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
/*�������ƣ�EEPROM_ReadArray 
/*�������ܣ��ӵ�ַsrc_addr��ʼ��ȡ����Ϊlen�����ݡ�װ��dest_addr�� 
/*��ڲ�����*dest_addr,src_addr,len
/*���ڲ����� 
/*�� ע�� 
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
/*�������ƣ�Timer3_ISR 
/*�������ܣ�����SCL�͵�ƽ��ʱ���,����SMBus 
/*��ڲ�������
/*���ڲ�������
/*�� ע�� 
/*************************************************************************/ 
void Timer3_ISR (void) interrupt 14
{
   SMB0CF &= ~0x80;                    // ��ֹSMBus
   SMB0CF |= 0x80;                     // ����SMBus
   TMR3CN &= ~0x80;                   
   SMB_BUSY = 0;                       
}
/*************************************************************************/
/*�������ƣ�SMBus_ISR 
/*�������ܣ����ڼ��SMBus�Ĺ���״̬,��������Ӧ�����ݴ���
/*��ڲ������� 
/*���ڲ������� 
/*�� ע�� 
/*************************************************************************/ 
void SMBus_ISR(void) interrupt 7
{
   bit FAIL = 0;                       //����ʧ�ܱ�־
   unsigned char temp;
   static char i;                      //���ͻ������������


   static bit SEND_START = 0;          //������־λ


   switch (SMB0CN & 0xF0)              // ״̬����
   {
      //�����ͻ����: ���������ѷ���
      case SMB_MTSTA:
         temp = (unsigned char)(WORD_ADDR>>8);
         temp &= 0x07;
   temp <<=1;                    //ȡ����ַ��8~10λ,�ϲ�������ѡ�������ֽ���
   SMB0DAT = TARGET|SMB_RW|temp; //װ��Ŀ���ַ
         STA = 0;                      //�ֶ��������λ
         i = 0;                        //��������λ������
         break;


      //������: ����λ��ӵ�ַ�ѷ���
      case SMB_MTDB:
         if(ACK)                       // �ӵ�ַ������λӦ��
         {                            
            if (SEND_START)
            {
               STA = 1;
               SEND_START = 0;
               break;
            }
            if(SMB_SENDWORDADDR)       //�Ƿ��������ڲ���ַ?
            {
               SMB_SENDWORDADDR = 0;   //���־λ
               SMB0DAT =(unsigned char)WORD_ADDR;    //���͵�8λ�����ڲ���ַ


               if (SMB_RANDOMREAD)
               {
                  SEND_START = 1;     
                  SMB_RW = READ;
               }


               break;
            }


            if (SMB_RW==WRITE)         //����˴η�����д����?
            {


               if (i < SMB_DATA_LEN)   //���ݷ���?
               {
                  //������
                  SMB0DAT = *pSMB_DATA_OUT;


                  //���͵����ݵ�ַ���ӣ�����д������
                  pSMB_DATA_OUT++;


                  //����������������һ
                  i++;
               }
               else
               {
                 STO = 1;              //��λSTO�Խ����˴δ���
                 SMB_BUSY = 0;         //���æ��־λ
               }
            }
            else {}                    //����˴��Ǹ�������,����ʾ����,
                                       
         }
         else                          //����յ�NACK,
         {
            if(SMB_ACKPOLL)
            {
               STA = 1;                //��������
            }
            else
            {
               FAIL = 1;              
            }                         
         }
         break;


      //������: �ֽڽ���
      case SMB_MRDB:
         if ( i < SMB_DATA_LEN )       //���û������?
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
   if (FAIL)                           //�������ʧ��
   {
      SMB0CF &= ~0x80;                
      SMB0CF |= 0x80;
      STA = 0;
      STO = 0;
      ACK = 0;
      SMB_BUSY = 0;                    //�ͷ�SMBus
      FAIL = 0;
   }
   SI = 0;                             //����жϱ�־λ
}
/*************************************************************************/
/*�������ƣ�main 
/*�������ܣ������� 
/*��ڲ������� 
/*���ڲ������� 
/*�� ע�� 
/*************************************************************************/ 
void main()
{
  unsigned char temp_char=0;
  bit error_flag = 0;
  char in_buff[8] = {0};             //�������ݼĴ���
  char out_buff[8] = "ABCDEFG";      //������ݼĴ���
  unsigned char i;
  PCA0MD &= ~0x40;                   //�رտ��Ź�
  Device_Init();                     //�豸��ʼ��
  WP = 0;
  EIE1 |= 0x01;                      //����SMBus�ж�
  EA = 1;                            //�������ж�
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

