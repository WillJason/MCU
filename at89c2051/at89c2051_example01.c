/*用AT89C2051的C语言程序，时间控制*/
#include<reg51.h>

#define HI (65536-49988)/256 /* 50ms run 1 time ISR */
#define LO (65536-49988)%256
#define DEDA 20/* 20*50ms=1sec */
#define n 10


/* 0-9的字型数据 */
char DATA_ASEG[10]={0x70,0x71,0x74,0x75,0x78,0x79,0x7c,0x7d,0x72,0x73};
/* 个位扫描控制信号 */
char DATA_BSEG[10]={0xb0,0xb1,0xb4,0xb5,0xb8,0xb9,0xbc,0xbd,0xb2,0xb3};
/* 十位扫描控制信号 */
char DATA_CSEG[10]={0xd0,0xd1,0xd4,0xd5,0xd8,0xd9,0xdc,0xdd,0xd2,0xd3};
/* 百位扫描控制信号 */
char DATA_DSEG[10]={0xe0,0xe1,0xe4,0xe5,0xe8,0xe9,0xec,0xed,0xe2,0xe3};
/* 千位扫描控制信号 */

char deda=0;
char min=0,sec=0,hour=0;
char z1=7,z2=12,z3=13,z4=17,z5=18,z6=21;
char zz1=59,zz2=0,zz3=29,zz4=30,zz5=29,zz6=30;
char jsq=0,jsq1=0,jsq2=0;
sbit P37=P3^0x07;
sbit P32=P3^0x02;
sbit P34=P3^0x04;
sbit P33=P3^0x03;
sbit P35=P3^0x05;
sbit P10=P1^0x00;
sbit P11=P1^0x01;
sbit P12=P1^0x02;
sbit P13=P1^0x03;

delay(int t)
 {
  int i,j;
  for(i=0;i<t;i++)
   for(j=0;j<5;j++);
 }
/*----------*/
  void timer0(void) interrupt 1
 {
 TR0=0;
 TH0=HI;
 TL0=LO;
 TR0=1;
 deda++;
 if(deda==DEDA){
 sec++;deda=0;
 }
 }

 /****************************/
init_timer()            /* 初始化计数器 */
{
/* 设定工作模式 */
TMOD=0x21;
TH0=HI;/*载入初值 */
TL0=LO;
IP=0x02;
IE=0x82;
TR0=1;
}
/*--------*/
sheding()                   //时间设定
{
   if(P34==0)jsq1++;
    else jsq1=0;
  if(jsq1>=30){min++;jsq1=0;}
       if(min==60){min=0;hour++;}
         if(hour==24)hour=0;

   if(P35==0)jsq2++;
    else jsq2=0;
  if(jsq2>=30){hour++;jsq2=0;}
       if(hour==24)hour=0;

 }
/******************************/
conv()                      /* 时间显示*/
{
P1=DATA_ASEG[min%10];
delay(n);

P1=DATA_BSEG[min/10];
delay(n);

P1=DATA_CSEG[hour%10];
delay(n);

P1=DATA_DSEG[hour/10];
delay(n);
}
/******************************/
main()                  /* 主程序 */
{
  sec=0;min=0;hour=0;
  conv();
 init_timer();                  /* 初始化计数器 */
 while(1){
 conv();
 if(sec==60){min++;sec=0;}
 if(min==60){hour++;min=0;}
 if(hour==24)hour=0;
 sheding();
 if((hour==z1&&min==zz1)|(hour==z2&&min==zz2)
   |(hour==z3&&min==zz3)|(hour==z4&&min==zz4)
   |(hour==z5&&min==zz5)|(hour==z6&&min==zz6)){P37=0;P33=0;}
 if(sec>=15){P37=1;P33=1;}
 }
}
