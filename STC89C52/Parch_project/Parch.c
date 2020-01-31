/*
	All programs are written in modularity,include DHT11 module.12864 module.
DS18B20 module and main program,Program design based on function block,The
 method to realize the algorithm is called modularization.The purpose of 
 modularization is to reduce the complexity of program and simplify the operation
  of program design, debug and maintenance.The main program first assigns Wendu 
  and Shidu variables to 0 to prevent data interference. Then delay for 1s, DHT11
   has error output in 1s of power on, so delay is used to not collect data and 
   other related operations. Then the 12864 is initialized and the initialization 
   function of 12864 module is called.After simple initialization, the program runs
    into the circulation program, initializes DS18B20 and completes a temperature
     conversion without taking other operations without data;Then call the read
      function in the DHT11 module program to read the measurement results of 
      the DHT11 temperature and humidity sensor and assign the result to variables
       Wendu and Shidu;Because 12864 has been initialized before, 12864 data display
        location function and display data value function can be called directly. 
        Wendu variable represents the temperature data, and the display address is
         0x94h (the 5th bit in the second line). Shidu variable represents the 
         humidity data, and the display address is 0x96h (the 7th bit in the 
         second line). The temperature and humidity program is completed;Then
          the DS18B20 temperature display function is called, and the two display
           function of 12864 is called. In order to give the response time of the
            sensor and facilitate the user's real-time monitoring, the last delay 
            of this cycle program is 1s.
*/
//main.c
#include<reg52.h>
#include<intrins.h>
#include"12864.h"
#include"ds18b20.h"
#include"11.h"
#define uchar unsigned char
#define uint unsigned int
sbit D1=P1^7;				//秒灯
sbit D4=P1^4;							//ce shi led
sbit heat=P1^2;			//加热器
sbit motor=P1^0;		//风扇
uint temp;					//variable of temperature
uchar flag1;				//sign of the result positive or negative
uchar wendu;
uchar shidu;
uchar heatwendu;						//wendu
uchar dhtwendu,dhtshidu;		//温湿度
uchar flag0,flasg1,flagmotor=0,sec,miao,fen,shi,tt=0,tt=1;
void handle();
void delay(uint count)				//delay
{
	uint i;
	while(count)
	{
		i=200;
		while(i>0)
		i--;
		count--;
	}
}
void delayms()
{	
	uchar i,j;
	for(i=0;i<100;i++)
		for(j=0;j<100;j++); 
}

void timer_init()
{
	TMOD=0x11;
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;
	TH1=(65536-50000)/256;
	TL1=(65536-50000)%256;
	ET0=1;
	ET1=1;
	EA=1;
	TR0=1;
	TR1=0;
	D1=1;
	miao=0;
	fen=0;
	shi=0;
	heat=0;
	motor=1;
}

main()
{
	wendu=0;
	shidu=0;
	delay1ms(1000);			//DHT11开始1s有错误输出
	init_12864();
	timer_init();
	while(1)
	{
		tmpchange();
		DHT11_DUSHU();
		write_pos(2,7);
		write_date(shidu/10%10+48);
		write_date(shidu%10+48);
		write_pos(2,5);
		write_date(wendu/10%10+48);
		write_date(wendu%10+48);
		display(tmp());
		heatwendu=tmp();
		dhtwendu=wendu;
		dhtshidu=shidu;
		handle();
		delay1ms(1000);
	}
}

void timer0() interrupt 1
{
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;
	tt0++;
	if(tt0==18)
	{
		tt0=0;
		flag0=1;
	}
}

void timer1() interrupt 3
{
	TH1=(65536-50000)/256;
	TL1=(65536-50000)%256;
	tt1++;
	if(tt1==180)
	{
		tt1=0;
		flag1=1;
	}
}

void handle()
{
	if(flag0==1)
	{
		flag0=0;
		D1=~D1;
		delayms();
		D1=~D1;
		miao++;
		if(miao==60)
		{
			miao=0;
			fen++;
			if(fen==60)
			{
				fen=0;
				shi++;
				if(shi==24)
				{
					shi=0;
				}
			}
		}
	}
	if(flag1==1)
	{
		flag1=0;
		sec++;
		if(sec==6)			//2min
		{
			sec=0;
			//D4=~D4;
			delayms();
			//D4=~D4;
			flagmotor=1;
		}
	}
	/******************************************************************************
	1.前期快速失水阶段：阶段时间1.5，加热温度上限40，空气闻湿度上限30/60%，以及等待时间2min
	00:01:00-01:35:00  1'34''
	2.短期恒湿阶段：阶段时间7.5,加热湿度上限55，空气温湿度上限45/85%,以及等待时间2min
	01:36:00-9:40:00 8'04''
	3.慢速失水阶段：阶段时间5，加热温度上限65，空气温湿度上限40/80%,以及等待时间2min
	09:41:00-14:55:00   5'14''
	ds18b20测量温度范围为-55℃至+125℃
	dht11测量范围：湿度20-90%RH，温度0~55℃
	风扇继电器 P1.0
	加热器继电器 P1.2
	*******************************************************************************/
	//第一阶段
	if((shi>=0&&fen>=0&&shi<1)||(shi>0&&shi<=1&&fen<=35))
	{
		if(heatwendu<35)
		{
			heat=0;			//开启
		}
		else
		{
			heat=1;
		}
		if(dhtwendu>30||dhtshidu>70)
		{
			TR1=1;
			if(flagmotor==1)
			{
				TR1=0;
				motor=0;
			}
		}
		else
		{
			TR1=0;
			flagmotor=0;
			motor=1;
		}
	}	
	else if((shi>=1&&fen>=36&&shi<9)||(shi>1&&shi<=9&&fen<40))	//第二阶段
	{
		if(heatwendu<50)
		{
			heat=0;		//开启
		}
		else
		{
			heat=1;
		}
		if(dhtwendu>45||dhtshidu>85)
		{
			TR1=1;
			if(flagmotor==1)
			{
				TR1=0;
				motor=0;
			}
		}
		else
		{
			TR1=0;
			flagmotor=0;
			motor=1;
		}
	}
	else if((shi>=9&&fen>=41&&shi<14)||(shi>9&&shi<=14&&fen<=55))		//第三阶段
	{
		if(heatwendu<60)
		{
			heat=0;		//开启
		}
		else
		{
			heat=1;
		}
		if(dhtwendu>40||dhtshidu>80)
		{
			TR1=1;
			if(flagmotor==1)
			{
				TR1=0;
				motor=0;
			}
		}
		else
		{
			TR1=0;
			flagmotor=0;
			motor=1;
		}
	}
	else
	{
		heat=0;
		motor=1;
	}
	/**********************************************************************************/
}









































