#include <REGX52.H>
sbit BUTTON=P3^2;
void delay_ms(unsigned int ms){unsigned int i,j;for(i=0;i<ms;i++)for(j=0;j<123;j++);}
void main(void){unsigned char mode=0;BUTTON=1;P1=0xFF;while(1){if(BUTTON==0){delay_ms(25);if(BUTTON==0){mode=(mode+1)%5;switch(mode){case 0:P1=0xFF;break;case 1:P1=0xFE;break;case 2:P1=0xFD;break;case 3:P1=0xFB;break;case 4:P1=0xF0;break;}while(BUTTON==0);delay_ms(25);}}}}
