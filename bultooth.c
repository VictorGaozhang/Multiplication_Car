#include <reg52.h>
#include <intrins.h> 
void ConfigUART();
void led(unsigned char n);
#define led_PORT P0

void ConfigUART()
{
	TMOD=0X20;//        Set T1 working mode to 8-bit auto-reload
	SCON=0X50;//        Set the serial port working mode to 10-bit asynchronous transceiver
	PCON=0X80;//        SMOD set to 1
	TH1=(256-26);
	TL1=(256-26);
	EA=1;//     Open total interrupt
	ES=1;//     Open serial port interrupt
	TR1=1;//        Start T1
}

void main()
{

	ConfigUART();
	
}

void led(unsigned char n)
{
	led_PORT = _crol_(0x01,n-1);
	led_PORT = ~led_PORT;
}


void Com_Int() interrupt 4
{
	if(RI==1)
	{
		char receive = SBUF;
		RI=0;
		switch (receive)
		{
			case 0x01:led(1);break;
			case 0x02:led(2);break;
		}
	}
}
