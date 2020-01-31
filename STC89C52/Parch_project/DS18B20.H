#ifndef _DS18B20_H_
#define _DS18B20_H_
#include"12864.h"
#define uchar unsigned char
#define uint unsigned int
sbit DS=P2^2;				//de
extern uint temp;			//v
void delay(uint count);		//delay
void dsreset(void);		//send reset and initialization command
bit tmpreadbit(void);		//read a bit
uchar tmpread(void);		//read a byte date
void tmpwritebyte(uchar dat);		//write a byte to ds18b20
void tmpchange(void);		//DS18B20 begin change
uint tmp();			//get the temperature
void readrom();		//read the serial
void delay10ms();	//delay
void display(uint temp);		//œ‘ æ≥Ã–Ú

#endif















