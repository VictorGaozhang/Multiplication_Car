#include <reg52.h>

#define TrackPort P2

sbit M2PWM = P1^0;
sbit M2B = P1^1;      
sbit M2A = P1^2;
sbit M1PWM = P1^5;
sbit M1B = P1^3;
sbit M1A = P1^4;

unsigned char timeCnt;
unsigned char DutyRatio_L,DutyRatio_R;
bit enableMotor = 0;
char Motor_L_e = 0;
char Motor_R_e = 0;



void ConfigUART();
void PWM_Config();
void setPWM(char set_L,char set_R);

unsigned char ScanLine();

//void delay(unsigned int t);
//void delay(unsigned int t)
//{
//	unsigned int i,j;
//	for(i=t;i>0;i--)
//		for(j=123;j>0;j--);
//}

void PWM_Config()
{
	TH2=0xFE;
	TL2=0x0c;
	ET2=1;
	EA=1;
	M2PWM=0;
	M1PWM =0;
	TR2=1;
}
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
void Com_Int() interrupt 4
{
	if(RI==1)
	{
		char receive = SBUF;
		RI=0;
		switch (receive)
		{
			case '1':enableMotor=~enableMotor;break;
		}
	}
}
void Timer2_PWM(void) interrupt 5
{
	TF2 =0;
	TH2=0xFE;
	TL2=0x0c;
	timeCnt++;
	if(timeCnt>100)
	{
		timeCnt=0;
	}
	if(enableMotor)
	{
		M2PWM = (timeCnt<DutyRatio_L)? 1 : 0;
		M1PWM = (timeCnt<DutyRatio_R)? 1 : 0;
	}
	else
	{
		M2PWM = 0;
		M1PWM = 0;
	}
}
void setPWM(char set_L,char set_R)
{
		
	if(set_L>=0)
	{
		M2B=0;
		M2A=1;
		DutyRatio_L = set_L + Motor_L_e;
	}
	else
	{
		M2B=1;
		M2A=0;
		DutyRatio_L = -set_L + Motor_L_e;
	}
	if(set_R>=0)
	{
		M1B=0;
		M1A=1;
		DutyRatio_R = set_R + Motor_R_e;
	}
	else
	{
		M1B=1;
		M1A=0;
		DutyRatio_R = -set_R + Motor_R_e;
	}
	if(DutyRatio_L > 100)
		DutyRatio_L = 100;
	if(DutyRatio_L < 0)
		DutyRatio_L = 0;
	if(DutyRatio_R > 100)
		DutyRatio_R = 100;
	if(DutyRatio_R < 0)
		DutyRatio_R = 0;
	
	
}
unsigned char ScanLine()
{
	unsigned char scanTemp;
	unsigned char scanResult;	

	scanTemp = TrackPort;
	scanTemp = TrackPort & 0x3F; //P2.0-P2.5

	switch(scanTemp)
	{
		case 0x00: scanResult = 0; break; //000000
		case 0x01: scanResult = 1; break; //000001
		default: scanResult = 8; break;		
	}
	return scanResult;
}
void main()
{
	PWM_Config();
	ConfigUART();
	while(1)
	{
		switch (ScanLine())
		{
			case(0):setPWM(60,60);break;
			case(1):setPWM(-50,50);break;
			
			
			case(8):setPWM(30,30);break;
		}
	}
}