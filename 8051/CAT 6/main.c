#include <REGX52.H>
#define N 32
volatile unsigned char buf[N],head=0,tail=0;
void uart_init(void){SCON=0x50;TMOD=(TMOD&0x0F)|0x20;TH1=0xFD;TL1=0xFD;TR1=1;ES=1;EA=1;}
void uart_isr(void) interrupt 4{unsigned char next;if(RI){RI=0;next=(head+1)%N;if(next!=tail){buf[head]=SBUF;head=next;}}if(TI)TI=0;}
void uart_send(unsigned char c){SBUF=c;while(!TI);TI=0;}
void main(void){unsigned char c;uart_init();while(1){if(tail!=head){c=buf[tail];tail=(tail+1)%N;uart_send(c);}}}
