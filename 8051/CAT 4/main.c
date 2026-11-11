#include <REGX52.H>
unsigned char code seg[10]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};
void delay_ms(unsigned int ms){unsigned int i,j;for(i=0;i<ms;i++)for(j=0;j<123;j++);}
unsigned char keyscan(void){unsigned char r,c;unsigned char code rows[4]={0xFE,0xFD,0xFB,0xF7};for(r=0;r<4;r++){P2=rows[r];if((P2&0xF0)!=0xF0){delay_ms(20);if(!(P2&0x10))c=0;else if(!(P2&0x20))c=1;else if(!(P2&0x40))c=2;else c=3;while((P2&0xF0)!=0xF0);return r*4+c;}}return 0xFF;}
void main(void){unsigned char k,i;P1=0xFF;P0=0xFF;P2=0xF0;while(1){k=keyscan();if(k!=0xFF){P0=seg[k%10];if(k<4){P1=0xFF;P1&=~(1<<k);}else if(k==4){for(i=0;i<4;i++){P1=0xFE;delay_ms(150);P1=0xFF;delay_ms(150);}}else if(k==5){P1=0xF5;delay_ms(200);P1=0xFA;delay_ms(200);}else if(k==6)P1=0xF0;else if(k==7){for(i=0;i<4;i++){P1=0xF0;delay_ms(200);P1=0xFF;delay_ms(200);}}else if(k==8||k==9)P1=0xFF;}}}
