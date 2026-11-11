#include <REGX52.H>
sbit OUT=P1^0;
sbit SELECT=P3^2;
volatile unsigned int half_period=500;
volatile unsigned int count=0;
void delay_ms(unsigned int ms){unsigned int i,j;for(i=0;i<ms;i++)for(j=0;j<123;j++);}
void timer0_init(void){TMOD=(TMOD&0xF0)|0x01;TH0=0xFC;TL0=0x66;ET0=1;EA=1;TR0=1;}
void timer0_isr(void) interrupt 1{TH0=0xFC;TL0=0x66;if(++count>=half_period){count=0;OUT=!OUT;}}
void main(void){unsigned char n=0;SELECT=1;timer0_init();while(1){if(SELECT==0){delay_ms(20);if(SELECT==0){n=(n+1)%5;switch(n){case 0:half_period=500;break;case 1:half_period=100;break;case 2:half_period=50;break;case 3:half_period=20;break;case 4:half_period=10;break;}while(SELECT==0);}}}}
