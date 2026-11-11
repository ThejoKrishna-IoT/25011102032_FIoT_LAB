#include <REGX52.H>
sbit LED=P1^0;
void delay_ms(unsigned int ms){unsigned int i,j;for(i=0;i<ms;i++)for(j=0;j<123;j++);}
void main(void){while(1){LED=0;delay_ms(500);LED=1;delay_ms(500);}}
