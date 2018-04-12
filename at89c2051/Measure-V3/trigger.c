/*
��Ƭ��ѡ��at89c2051���˿�P1.2Ϊ�����ⲿ��ʱ����ZF5W-
2��ʱ����������ʱ�˿ڣ��ⲿ�жϵ͵�ƽ����������һ��
��ʼ��ʱ���ٴδ�����ֹͣ��ʱ��
�°汾Ϊ˫�������ⲿ�ж�1Ϊ��ʼ��ʱ�������ⲿ�ж�0Ϊֹͣ��ʱ������δ�����ж�ֹͣ��ʱ��
����ѡ���ܣ�ѡ��MAX7219Ϊ����������������������
*/
#include<REG2051.H>

#define Timer_H (65536-50000)/256 												/* 50ms run 1 time ISR */
#define Timer_L (65536-50000)%256
//#define DEDA 20																					/* 20*50ms=1sec */
//#define n 10    																				/*LED��ʾ��ʱ*/

sbit LOAD=P1^5;  																					//MAX7219Ƭѡ
sbit DIN =P1^7;  																					//MAX7219��������
sbit CLK =P1^6;  																					//MAX7219����ʱ��
#define DECODE_MODE 0x09 																	//���������
#define INTENSITY   0x0A   																//���ȿ�����
#define SCAN_LIMIT  0x0B   																//ɨ��߽������
#define SHUT_DOWN   0x0C  															 	//�ض�ģʽ�Ĵ���
#define DISPLAY_TEST 0x0F   															//���Կ��ƼĴ���

char Trig_Num;
int Record_Num;
int Record_Num_Hundred;
int Record_Num_Decade;
int Record_Num_Ones;
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
		LOAD =0;             																	//����Ƭѡ�ߣ�ѡ������
		//���͵�ַ
	  for(i=0;i<8;i++)		 																	//��λѭ��8��
		{
				CLK =0;           																//����ʱ������
				DIN =(bit)(address&0x80);													//ÿ��ȡ���ֽ�
				address<<=1;																			//����һλ
				CLK =1;																						//ʱ�������أ����͵�ַ
		}
		//��������
		for(i=0;i<8;i++)
		{
				CLK=0;
				DIN=(bit)(dat&0x80);
				dat<<=1;
				CLK =1;             															//ʱ�������أ���������
		}
		LOAD=1;                 															//���ͽ�������������������
}	
//MAX7219��ʼ��������MAX7219�ڲ��Ŀ��ƼĴ���
void Initial(void)
{
		Write7219(SHUT_DOWN,0x01);         										//������������ģʽ(0xX1)
		Write7219(DISPLAY_TEST,0x00);      										//ѡ����ģʽ(0xX0)
		Write7219(DECODE_MODE,0x0f);       										//ѡ�ð�����ģʽ
		Write7219(SCAN_LIMIT,0x02);        										//3ֻLED
		Write7219(INTENSITY,0x04);          									//���ó�ʼ������
}
void delay(int t)
 {
  int i,j;
  for(i=0;i<t;i++)
  for(j=0;j<5;j++);

 }

 void Ex_0(void) interrupt 0
{
		EX0=0; 																								//�ر��ⲿ�ж�
		if(Trig_Num!=0)																				//������ڼ�ʱ״̬
		{
			//TR0=1;
			P1_2=1;
			Trig_Num=0;																					//ֹͣ��ʱ������δ��ʱ״̬����������
			Record_Num++;																				//���һ�μ�ʱ����¼����ʾ
		}                    																	
		else
		{}            
}

void Ex_1(void) interrupt 2
{
		EX1=0;                                                //�ر��ⲿ�ж�
		if(Trig_Num==0)																				//�������δ��ʱ״̬
		{
			//TR0=1;
			P1_2=0;
			Trig_Num++;																					//��ʼ��ʱ�����ڼ�ʱ״̬����������
		}                    																	
		else{}
}
 /****************************/
void init_Ex_Timer()            													/* ?????? */
{
/* ?????? */
	TMOD=0x01;
	//EX0=1;
	//IT0=0;
	
	EX1=1;
	IT1=0;
	EA=1;
}

void main()                                               /* ������*/
{
	Record_Num_Hundred=0;
	Record_Num_Decade=0;
	Record_Num_Ones=0;
	Record_Num=0;
	Trig_Num=0;																							//Ĭ��Ϊ��һ�δ���
	init_Ex_Timer();                                       	/* ��ʼ�� �ⲿ�ж�1 ���͵�ƽ�����Ͷ�ʱ��0*/
	//Initial();               																//MAX7219???
	while(1)
	{
			if(P3_3!=0){EX1=1;}                                 //��ֹ�͵�ƽ��������
			//if(P3_2!=0){EX0=1;}                                 //��ֹ�͵�ƽ��������
			//Record_Num_Hundred=Record_Num/100;
		///	Record_Num_Decade=(Record_Num%100)/10;
		//	Record_Num_Ones=(Record_Num%100)%10;
		//	Write7219(1,Record_Num_Hundred);      							//��ʾ��λ����
		//	Write7219(2,Record_Num_Decade);      								//��ʾʮλ����
		//	Write7219(3,(Record_Num_Ones|0x80));      					//��ʾ��λ����
			if(P3_2!=0)
			{
					if(Trig_Num!=0)																				//������ڼ�ʱ״̬
					{
						//TR0=1;
						P1_2=1;
						Trig_Num=0;																					//ֹͣ��ʱ������δ��ʱ״̬����������
						//Record_Num++;																				//���һ�μ�ʱ����¼����ʾ
					}  
					else
					{}
			}
			else
			{}
	}
}
