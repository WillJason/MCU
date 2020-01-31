#include<reg52.h>
#include"12864.h"
void delay1ms(uint z)
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=114;y>0;y--);
}
void write_com(uchar com)
{
	lcdrs=0;
	P0=com;
	delay1ms(1);
	lcden=1;
	delay1ms(1);
	lcden=0;
}
void write_date(uchar date)
{
	lcdrs=1;
	P0=date;
	delay1ms(1);
	lcden=1;
	delay1ms(1);
	lcden=0;
}
void write_str(uchar *str)
{
	while(*str!='\0')	//未结束
	{
		write_date(*str++);
		delay1ms(5);
	}
}
void write_pos(uchar x,uchar y)	//从第X行的第Y位置开始显示
{
	uchar pos;
	if(x==1)	//第一行
	{x=0x80;}
	else if(x==2)	//第二行
	{x=0x90;}
	else if(x==3)	//第三行
	{x=0x88;}
	else if(x==4)	//第四行
	{x=0x98;}
	pos=x+y-1;	//首地址为0x80
	write_com(pos);
}
void init_12864()
{
	lcdrw=0;
	lcden=0;
	write_com(0x01);
	delay1ms(5);
	write_com(0x30);
	write_com(0x06);
	write_com(0x0c);
	write_com(0x88);
	write_pos(1,1);
	write_str("某某干燥控制系统");
	write_pos(2,1);
	write_str("温湿度：");
	write_pos(2,8);
	write_str("%");
	write_pos(3,1);
	write_str("当前温度：");
	write_pos(2,6);
	write_str("度");
	write_pos(3,8);
	write_str("度");
	write_pos(4,1);
	write_str("某某工作单位");
}







































