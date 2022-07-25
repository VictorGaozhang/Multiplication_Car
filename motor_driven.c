#include <reg52.h>

sbit M2PWM = P1^0;		 //Left motor IO port
sbit M2B = P1^1;      
sbit M2A = P1^2;
sbit M1PWM = P1^5;		 //Right motor IO port
sbit M1B = P1^3;
sbit M1A = P1^4;

unsigned char timeCnt;
char DutyRatio_L,DutyRatio_R;

void PWM_Config();
void setPWM(char set_L,char set_R);
void delay(unsigned int t);
void delay(unsigned int t)
{
	unsigned int i,j;
	for(i=t;i>0;i--)
		for(j=123;j>0;j--);
}
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
	M2PWM = (timeCnt<DutyRatio_L)? 1 : 0;
	M1PWM = (timeCnt<DutyRatio_R)? 1 : 0;
}
void setPWM(char set_L,char set_R)
{
	if(set_L>=0)
	{
		M2B=0;
		M2A=1;
		DutyRatio_L = set_L;
	}
	else
	{
		M2B=1;
		M2A=0;
		DutyRatio_L = -set_L;
	}
	if(set_R>=0)
	{
		M1B=0;
		M1A=1;
		DutyRatio_R = set_R;
	}
	else
	{
		M1B=1;
		M1A=0;
		DutyRatio_R = -set_R;
	}
}


unsigned char KeyDown()
{
	unsigned char KeyValue;
	P2=0x0f;
	if(P2!=0x0f)
	{
		delay(10);
		if(P2!=0x0f)
		{
			P2=0X0F;
			switch(P2)
			{
				case(0X07):KeyValue=1;break;
		    	case(0X0b):KeyValue=2;break;
		    	case(0X0d):KeyValue=3;break;
		    	case(0X0e):KeyValue=4;break;
			}
			P2=0XF0;
			switch(P2)
			{
	        	case(0X70):KeyValue=KeyValue;break;
	        	case(0Xb0):KeyValue=KeyValue+4;break;
		    	case(0Xd0):KeyValue=KeyValue+8;break;
		    	case(0Xe0):KeyValue=KeyValue+12;break;
	        }
			while(P2!=0xf0)
			{
				P2=0xf0;
				delay(5);
			}
		}
	}
	return KeyValue;
}

void main()
{
	PWM_Config();
	
	while(1)
	{
		switch (KeyDown())
		{
			case(1):setPWM(10,10);break;
			case(2):setPWM(20,20);break;
			case(3):setPWM(30,30);break;
			case(10):setPWM(100,100);break;
		}
	}
}