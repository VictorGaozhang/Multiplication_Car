#include <reg52.h>

#define TrackPort P2  //Tracking module interface
#define ObstaclePort P0 //Obstacle Avoidance Module Interface

//Motor drive interface
sbit M2PWM = P1^0;
sbit M2B = P1^1;      
sbit M2A = P1^2;
sbit M1PWM = P1^5;
sbit M1B = P1^3;
sbit M1A = P1^4;

unsigned char timeCnt; //Cycle count value
unsigned char DutyRatio_L,DutyRatio_R; //duty cycle£º 0-100

char Motor_L_e = 0; //mechanical error
char Motor_R_e = 0;

bit Mode = 0; //Mode switch 0 Tracking module 1 Obstacle Avoidance Module
bit enableMotor = 1; //Motor start switch

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

void PWM_Config()  //Timer initialization
{
	TH2=0xFE;
	TL2=0x0c;
	ET2=1;
	EA=1;
	M2PWM=0;
	M1PWM =0;
	TR2=1;
}
void ConfigUART()  //Serial port initialization 12M 2400
{
	TMOD=0X20;//Set T1 working mode to 8-bit auto-reload
	SCON=0X50;//Set the serial port working mode to 10-bit asynchronous transceiver
	PCON=0X80;//SMOD set to 1
	TH1=(256-26);
	TL1=(256-26);
	EA=1;//switch interrupt
	ES=1;//Open serial port interrupt
	TR1=1;//Start T1
}
void Com_Int() interrupt 4  //Serial port interrupt function
{
	if(RI==1)
	{
		char receive = SBUF;
		RI=0;
		
		//Bluetooth module receives data processing
		switch (receive)
		{
			case '1':  //Motor start switch toggle
				enableMotor=~enableMotor;break;
			case '2':   //Obstacle avoidance tracking mode switch
				Mode=~Mode;break;
			//case 1 + 0x30 :enableMotor=~enableMotor;break;
		}
	}
}
void Timer2_PWM(void) interrupt 5  //Timer interrupt function
{
	TF2 =0;
	TH2=0xFE;
	TL2=0x0c;
	
	timeCnt++; //counting
	if(timeCnt>100)
	{
		timeCnt=0;  //Counting period 100 resets to zero
	}
	if(enableMotor)  //Motor start switch
	{
		//starting
		M2PWM = (timeCnt<DutyRatio_L)? 1 : 0;
		M1PWM = (timeCnt<DutyRatio_R)? 1 : 0;
	}
	else
	{
		//closing
		M2PWM = 0;
		M1PWM = 0;
	}
}
void setPWM(char set_L,char set_R)
{
		
	if(set_L>=0)
	{
		//Forward
		M2B=0;
		M2A=1;
		//Set duty cycle, Motor_L_e is mechanical error correction
		DutyRatio_L = set_L + Motor_L_e;
	}
	else
	{
		//Backward
		M2B=1;
		M2A=0;
		//Set the duty cycle (the negative sign represents the absolute value) Motor_L_e is the mechanical error correction
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
	
	//Limiting
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
	unsigned char scanResult;	 //return value

	scanTemp = TrackPort;   //read TrackPort
	scanTemp = TrackPort & 0x3F; //Extract Px.0-Px.5 in TrackPort

	switch(scanTemp)
	{
		case 0x3f: scanResult = 1; break; //111111
		case 0x3e: scanResult = 2; break; //111110
		default: scanResult = 0; break;		//others
	}
	return scanResult;  //return final value
}

unsigned char ScanObstacle()  //Obstacle avoidance detection
{
	unsigned char scanTemp; 
	unsigned char scanResult;	 //return value

	scanTemp = ObstaclePort;  //read ObstaclePort
	scanTemp = ObstaclePort & 0x1F; //Extract Px.0-Px.4 in ObstaclePort

	switch(scanTemp)
	{
		case 0x0f: scanResult = 1; break; //01111 
		case 0x1e: scanResult = 2; break; //11110
		
		default: scanResult = 0; break;//others
	}
	return scanResult; //return final value
}


void main()
{
	PWM_Config(); //PWM timer initialization
	ConfigUART(); //Serial port initialization 12M 2400
	while(1)
	{
		if(Mode) //Mode 1 Obstacle avoidance
		{
			switch (ScanObstacle())
			{
				case(1):setPWM(60,30);break; //Turn right slightly
				case(2):setPWM(30,60);break; //Turn left slightly
								
				case(0):setPWM(30,30);break; //go forward slightly
			}
		}
		else  // Mode 0 tracking
		{
			switch (ScanLine())
			{
				case(1):setPWM(60,60);break;   //go forward
				case(2):setPWM(-50,50);break;  //turn left
				
				
				case(0):setPWM(30,30);break;  //go forward slightly
			}
		}
	}
}