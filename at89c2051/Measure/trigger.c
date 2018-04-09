/*
��Ƭ��ѡ��at89c2051���˿�P1.2Ϊ�����ⲿ��ʱ����ZF5W-
2��ʱ����������ʱ�˿ڣ��ⲿ�жϵ͵�ƽ����������һ��
��ʼ��ʱ���ٴδ�����ֹͣ��ʱ��
����ѡ���ܣ�ѡ��MAX7219Ϊ����������������������
*/
#include<REG2051.H>

#define Timer_H (65536-50000)/256 /* 50ms run 1 time ISR */
#define Timer_L (65536-50000)%256
//#define DEDA 20/* 20*50ms=1sec */
//#define n 10    /*LED��ʾ��ʱ*/

sbit LOAD=P1^5;  //MAX7219Ƭѡ
sbit DIN =P1^7;  //MAX7219��������
sbit CLK =P1^6;  //MAX7219����ʱ��
#define DECODE_MODE 0x09 //���������
#define INTENSITY   0x0A   //���ȿ�����
#define SCAN_LIMIT  0x0B   //ɨ��߽������
#define SHUT_DOWN   0x0C   //�ض�ģʽ�Ĵ���
#define DISPLAY_TEST 0x0F   //���Կ��ƼĴ���

char Trig_Num;
//int Timer50ms_Num;
//int FinishMeasureTime;
unsigned char i;

//��������
void Write7219(unsigned char address,unsigned char dat);
void Initial(void);

//��ַ�����ݷ����ӳ���
void Write7219(unsigned char address,unsigned char dat)
{
		unsigned char i;
		LOAD =0;             //����Ƭѡ�ߣ�ѡ������
		//���͵�ַ
	  for(i=0;i<8;i++)		 //��λѭ��8��
		{
				CLK =0;           //����ʱ������
				DIN =(bit)(address&0x80);//ÿ��ȡ���ֽ�
				address<<=1;			//����һλ
				CLK =1;					//ʱ�������أ����͵�ַ
		}
		//��������
		for(i=0;i<8;i++)
		{
				CLK=0;
				DIN=(bit)(dat&0x80);
				dat<<=1;
				CLK =1;             //ʱ�������أ���������
		}
		LOAD=1;                 //���ͽ�������������������
}	
//MAX7219��ʼ��������MAX7219�ڲ��Ŀ��ƼĴ���
void Initial(void)
{
		Write7219(SHUT_DOWN,0x01);         //������������ģʽ(0xX1)
		Write7219(DISPLAY_TEST,0x00);      //ѡ����ģʽ(0xX0)
		Write7219(DECODE_MODE,0x0f);       //ѡ�ð�����ģʽ
		Write7219(SCAN_LIMIT,0x02);        //3ֻLED
		Write7219(INTENSITY,0x04);          //���ó�ʼ������
}
void delay(int t)
 {
  int i,j;
  for(i=0;i<t;i++)
  for(j=0;j<5;j++);

 }
/*----------*/
/*void Timer0(void) interrupt 1
{
	 TR0=0;
	 TH0=Timer_H;
	 TL0=Timer_L;																					  
	 Timer50ms_Num++;																				//���һ��50ms��ʱ
	 TR0=1;
	 
}*/

void Ex_1(void) interrupt 2
{
		EX1=0;                                                //�ر��ⲿ�ж�
		if(Trig_Num==0)
		{
			//TR0=1;
			P1_2=0;
			Trig_Num++;}                    										//�������δ���ڼ�ʱ״̬��ʼ��ʱ
		else
		{																											//������ڴ��ڼ�ʱ״̬��ֹͣ��ʱ
			//TR0=0;
			P1_2=1;
			Trig_Num=0;
			//FinishMeasureTime=Timer50ms_Num*50000+(50000-(TH0*256+TL0));              //���ղ�õĴ���ʱ���
			//Timer50ms_Num=0;
		}                               
		//EX1=1;
}
 /****************************/
void init_Ex_Timer()            													/* ?????? */
{
/* ?????? */
	TMOD=0x01;
//	TH0=Timer_H;                  													/*???? */
//	TL0=Timer_L;
//	IP=0x06;
//	ET0=1;
//	TR0=0;

	EX1=1;
	IT1=0;
	EA=1;
}

void main()                                               /* ������*/
{
	Trig_Num=0;                                             //Ĭ��Ϊ��һ�δ���
	//Timer50ms_Num=0;
	init_Ex_Timer();                                       /* ��ʼ�� �ⲿ�ж�1 ���͵�ƽ�����Ͷ�ʱ��0*/
	//Initial();               //MAX7219???
	while(1)
	{
			if(P3_3!=0){EX1=1;}                                 //���ⲿ�ж�����Ϊ�ߵ�ƽʱʹ���ⲿ�ж�

	}
}
