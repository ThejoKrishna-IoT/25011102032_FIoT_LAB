#include <REGX52.H>
sbit RS=P1^0;sbit EN=P1^1;sbit D4=P1^4;sbit D5=P1^5;sbit D6=P1^6;sbit D7=P1^7;
unsigned char x=0,y=0;
void delay_ms(unsigned int ms){unsigned int i,j;for(i=0;i<ms;i++)for(j=0;j<123;j++);}
void nib(unsigned char n){D4=n&1;D5=(n>>1)&1;D6=(n>>2)&1;D7=(n>>3)&1;EN=1;delay_ms(1);EN=0;}
void lcd(unsigned char v,bit d){RS=d;nib(v>>4);nib(v&15);delay_ms(2);}
void cmd(unsigned char c){lcd(c,0);}void dat(unsigned char c){lcd(c,1);}
void init(void){delay_ms(20);nib(3);delay_ms(5);nib(3);delay_ms(2);nib(3);nib(2);cmd(0x28);cmd(0x0C);cmd(0x06);cmd(1);}
unsigned char scan(void){unsigned char r;unsigned char code p[4]={0xFE,0xFD,0xFB,0xF7};for(r=0;r<4;r++){P2=p[r];if((P2&0xF0)!=0xF0){delay_ms(20);if(!(P2&0x10))return '1';if(!(P2&0x20))return '2';if(!(P2&0x40))return '3';if(!(P2&0x80))return 'A';while((P2&0xF0)!=0xF0);}}return 0;}
void main(void){unsigned char k;init();cmd(0x80);dat('U');dat('S');dat('E');dat(' ');dat('2');dat('/');dat('8');dat('/');dat('4');dat('/');dat('6');while(1){k=scan();if(k){if(k=='2'&&y>0)y--;if(k=='8'&&y<1)y++;if(k=='4'&&x>0)x--;if(k=='6'&&x<15)x++;cmd(0x01);cmd(0x80+y*0x40+x);dat('*');delay_ms(120);}}}
