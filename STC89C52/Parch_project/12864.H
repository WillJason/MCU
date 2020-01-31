#ifndef _12864_H_
#define _12864_H_
#define uchar unsigned char
#define uint unsigned int
sbit lcden=P3^4;
sbit lcdrs=P3^5;
sbit lcdrw=P3^6;
void delay1ms(uint z);
void write_com(uchar com);
void write_date(uchar date);
void write_str(uchar *str);
void write_pos(uchar x,uchar y);	//从第x行的第y位置开始显示
void init_12864();
#endif
























































