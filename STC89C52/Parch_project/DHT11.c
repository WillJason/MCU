#include<reg52.h>
#include"12864.h"
#include"11.h"
bit init_DHT11()
{
	bit flag;
	uchar num;
	DQ=0;
	delay1ms(19);	//>18ms
	DQ=1;
	for(num=0;num<10;num++);	//20-40us  34.7us
	for(num=0;num<11;num++);
	flag=DQ;
	for(num=0;num<11;num++);	//DHT响应80us
	for(num=0;num<24;num++);	//DHT拉高80us
	return flag;
}
uchar DHT11_RD_CHAR()
{
	uchar byte=0;
	uchar num;
	uchar num1;
	while(DQ==1);
	for(num1=0;num1<8;num1++)
	{
		while(DQ==0);
		byte<<=1;		//高位在前
		for(num=0;DQ==1;num++);
		if(num<10)
			byte|=0x00;
		else
			byte|=0x01;
	}
	return byte;
}
void DHT11_DUSHU()
{
	uchar num;
	if(init_DHT11()==0)
	{
		shidu=DHT11_RD_CHAR()-2;	//比正常值高2度左右
		DHT11_RD_CHAR();
		wendu=DHT11_RD_CHAR();
		DHT11_RD_CHAR();
		DHT11_RD_CHAR();
		for(num=0;num<17;num++);	//最后BIT输出后拉低总线 59us
		DQ=1;
		BELL=0;
		delay1ms(1);
		BELL=1;
	}
}











