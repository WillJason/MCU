#include<reg52.h>
#include"12864.h"
#include"ds18b20.h"
void dsreset(void)	//send reset and initialization command
{
	uint i;
	DS=0;
	i=103;
	while(i>0)i--;
	DS=1;
	i=4;
	while(i>0)i--;
}

bit tmpreadbit(void)	//read a bit
{
	uint i;
	bit dat;
	DS=0;i++;						//i++ for delay
	DS=1;i++;i++;
	dat=DS;
	i=8;while(i>0)i--;
	return(dat);
}

uchar tmpread(void)	//read a byte date
{
	uchar i,j,dat;
	dat=0;
	for(i=1;i<=8;i++)
	{
		j=tmpreadbit();
		dat=(j<<7)|(dat>>1);	//读出的数据最低位在最前面，这样刚好一个字节在DAT里
	}
	return(dat);
}

void tmpwritebyte(uchar dat)	//write a byte to ds18b20
{
	uint i;
	uchar j;
	bit testb;
	for(j=1;j<=8;j++)
	{
		testb=dat&0x01;
		dat=dat>>1;
		if(testb)		//wrtie 1
		{
			DS=0;
			i++;i++;
			DS=1;
			i=8;while(i>0)i--;
		}
		else
		{
			DS=0;			//write 0
			i=8;while(i>0)i--;
			DS=1;
			i++;i++;
		}
	}
}
void tmpchange(void)	//DS18B20 begin change
{
	dsreset();
	delay(1);
	tmpwritebyte(0xcc);	//address all drivers on bus
	tmpwritebyte(0x44);	//initiates a temperature conversion
}
uint tmp()						//get the temperature
{
	float tt;
	uchar a,b;
	dsreset();
	delay(1);
	tmpwritebyte(0xcc);
	tmpwritebyte(0xbe);
	a=tmpread();
	b=tmpread();
	temp=b;
	temp<<=8;			//two byte compose a int variable
	temp=temp|a;
	tt=temp*0.0625;
	temp=tt*10+0.5;
	return temp;
}

void readrom()		//read the serial
{
	uchar sn1,sn2;
	dsreset();
	delay(1);
	tmpwritebyte(0x33);
	sn1=tmpread();
	sn2=tmpread();
}
void delay10ms()		//delay
{
	uchar a,b;
	for(a=10;a>0;a--)
		for(b=60;b>0;b--);
}
void display(uint temp)			//显示程序
{
	uchar A1,A2,A2t,A3;
	//ser=temp/10;
	//SBUF=ser;
	A1=temp/100;
	A2t=temp%100;
	A2=A2t/10;
	A3=A2t%10;
	write_pos(3,6);
	write_date(A1+48);
	write_date(A2+48);
	write_pos(3,7);
	write_date(46);
	write_date(A3+48);
}









































