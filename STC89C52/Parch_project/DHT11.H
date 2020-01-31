#ifndef _11_H_
#define _11_H_
#include"12864.h"
#define uchar unsigned char
#define uint unsigned int

extern uchar wendu;
extern uint shidu;
sbit DQ=P2^5;
sbit BELL=P2^3;
bit init_DHT11();
uchar DHT11_RD_CHAR();
extern void DHT11_DUSHU();
#endif











