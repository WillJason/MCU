/*
单片机选用at89c2051，端口P1.2为触发外部计时器（ZF5W-
2计时计数器）计时端口，外部中断低电平触发，触发一次
开始计时；再次触发，停止计时。
新版本为双触发：外部中断1为开始计时触发，外部中断0为停止计时触发（未进入中断停止计时）
附加选择功能：选用MAX7219为数码管驱动器，驱动数码管
*/
#include<REG2051.H>

#define Timer_H (65536-50000)/256 												/* 50ms run 1 time ISR */
#define Timer_L (65536-50000)%256
//#define DEDA 20																					/* 20*50ms=1sec */
//#define n 10    																				/*LED显示延时*/

sbit LOAD=P1^5;  																					//MAX7219片选
sbit DIN =P1^7;  																					//MAX7219串行数据
sbit CLK =P1^6;  																					//MAX7219串行时钟
#define DECODE_MODE 0x09 																	//译码控制器
#define INTENSITY   0x0A   																//亮度控制器
#define SCAN_LIMIT  0x0B   																//扫描边界控制器
#define SHUT_DOWN   0x0C  															 	//关断模式寄存器
#define DISPLAY_TEST 0x0F   															//测试控制寄存器

char Trig_Num;
int Record_Num;
int Record_Num_Hundred;
int Record_Num_Decade;
int Record_Num_Ones;
//int Timer50ms_Num;
//int FinishMeasureTime;
unsigned char i;

//函数声明
void Write7219(unsigned char address,unsigned char dat);
void Initial(void);

//地址、数据发送子程序
void Write7219(unsigned char address,unsigned char dat)
{
		unsigned char i;
		LOAD =0;             																	//拉低片选线，选中器件
		//发送地址
	  for(i=0;i<8;i++)		 																	//移位循环8次
		{
				CLK =0;           																//清零时钟总线
				DIN =(bit)(address&0x80);													//每次取高字节
				address<<=1;																			//左移一位
				CLK =1;																						//时钟上升沿，发送地址
		}
		//发送数据
		for(i=0;i<8;i++)
		{
				CLK=0;
				DIN=(bit)(dat&0x80);
				dat<<=1;
				CLK =1;             															//时钟上升沿，发送数据
		}
		LOAD=1;                 															//发送结束，上升沿锁存数据
}	
//MAX7219初始化，设置MAX7219内部的控制寄存器
void Initial(void)
{
		Write7219(SHUT_DOWN,0x01);         										//开启正常工作模式(0xX1)
		Write7219(DISPLAY_TEST,0x00);      										//选择工作模式(0xX0)
		Write7219(DECODE_MODE,0x0f);       										//选用半译码模式
		Write7219(SCAN_LIMIT,0x02);        										//3只LED
		Write7219(INTENSITY,0x04);          									//设置初始化亮度
}
void delay(int t)
 {
  int i,j;
  for(i=0;i<t;i++)
  for(j=0;j<5;j++);

 }

 void Ex_0(void) interrupt 0
{
		EX0=0; 																								//关闭外部中断
		if(Trig_Num!=0)																				//如果处于计时状态
		{
			//TR0=1;
			P1_2=1;
			Trig_Num=0;																					//停止计时，处于未计时状态（接下来）
			Record_Num++;																				//完成一次计时，记录并显示
		}                    																	
		else
		{}            
}

void Ex_1(void) interrupt 2
{
		EX1=0;                                                //关闭外部中断
		if(Trig_Num==0)																				//如果处于未计时状态
		{
			//TR0=1;
			P1_2=0;
			Trig_Num++;																					//开始计时，处于计时状态（接下来）
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

void main()                                               /* 主程序*/
{
	Record_Num_Hundred=0;
	Record_Num_Decade=0;
	Record_Num_Ones=0;
	Record_Num=0;
	Trig_Num=0;																							//默认为第一次触发
	init_Ex_Timer();                                       	/* 初始化 外部中断1 ：低电平触发和定时器0*/
	//Initial();               																//MAX7219???
	while(1)
	{
			if(P3_3!=0){EX1=1;}                                 //防止低电平连续触发
			//if(P3_2!=0){EX0=1;}                                 //防止低电平连续触发
			//Record_Num_Hundred=Record_Num/100;
		///	Record_Num_Decade=(Record_Num%100)/10;
		//	Record_Num_Ones=(Record_Num%100)%10;
		//	Write7219(1,Record_Num_Hundred);      							//显示百位数字
		//	Write7219(2,Record_Num_Decade);      								//显示十位数字
		//	Write7219(3,(Record_Num_Ones|0x80));      					//显示个位数字
			if(P3_2!=0)
			{
					if(Trig_Num!=0)																				//如果处于计时状态
					{
						//TR0=1;
						P1_2=1;
						Trig_Num=0;																					//停止计时，处于未计时状态（接下来）
						//Record_Num++;																				//完成一次计时，记录并显示
					}  
					else
					{}
			}
			else
			{}
	}
}
