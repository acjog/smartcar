#include <reg51.h>

sbit LED = P1^1;
sbit MOTOR1A = P2^2;
sbit MOTOR1B = P2^3;

sbit MOTOR2A = P2^4;
sbit MOTOR2B = P2^5;

sbit TEST = P1^2;

char *CMD_MODE = "AT+CWMODE=3\r\n";
char *CMD_RST = "AT+RST\r\n";
char *CMD_WIFI = "AT+CWJAP=\"HUAWEI-XGYYA3\",\"lhhy604wifi\"\r\n";
char *CMD_SERVER = "AT+CIPSERVER=1\r\n";
char *CMD_MUX = "AT+CIPMUX=1\r\n";

unsigned char uartbuf[20+1];
unsigned int recv_num = 0;
unsigned int recv_flag = 0;
unsigned int send_ready = 0;

void Usart_init()
{
   SCON=0x50;  //选择工作方式1，允许串行口接收数据  
   TMOD=0x20;  //选择8位自动重装定时器
   PCON=0x80;  //波特率增倍
   TH1=0xFA;   //晶振频率11.0592MHZ，SMOD=1
   TL1=0xFA;   //波特率为9600时定时器的初值
   TR1=1;      //开定时器

   EA = 1; //开总中断
   ES = 1; //开串口中断
}

void send_char(unsigned char ch)
{
    ES=0; //发送期间关闭串口中断
    /*单片机发送数据*/
    SBUF=ch;   //将数据写入发送缓冲区中
    while(!TI);  //等待发送完毕
    TI=0;        //清除发送标志位
    ES=1;   //发送完成开串口中断
}

void send_string(unsigned char *str)
{
	unsigned char *p = str;
	for (; *p != '\0'; ++p)
	{
		send_char(*p);
	}
}


void delay_ms(unsigned int value)
{
	unsigned int i,j;
	for(i = 0; i < value; i++)
	{
		  for (j = 0; j < 123; j++);
	}
}

void ser() interrupt 4 
{
	unsigned char tmp;
	if (RI)
	{
		ES = 0;
		RI = 0;
		tmp = SBUF;
		if (recv_num < 20)
		{
			//+IPD,n,<string.length> 
			if (tmp == '+')
			{
		 		recv_num = 0;
			}
			uartbuf[recv_num] = tmp;
		 	recv_num ++;
		}
		else
		{
			recv_num = 0;
		}

		if (uartbuf[0] == '+')
		{
			recv_flag = 1;
			send_ready = 0;
		}
		else
		{
			recv_flag = 0;
		}
		if (uartbuf[0] == '>')
		{
			send_ready = 1;
		} 
		ES = 1;
	}
}

 /*
int check_ok()
{
	while(recv_num < 2);
	
	if ( BUF[0] == 'o' && BUF[1] == 'k')
	{
		return 1;
	}
	return 0;
}*/

void get_data()						  
{
	unsigned char cmd;
	if (recv_flag)
	{
		//+IPD,0,1:c
		if (recv_num >= 10 )
		{
			recv_flag = 0;
			cmd = uartbuf[9];
			switch ( cmd )
			{
				case (unsigned char)'L':
					{
						MOTOR1A = 1;
						MOTOR1B	= 1;

						MOTOR2A = 1;
						MOTOR2B	= 0;
						break;
					}
				case 'R':
					{
					    MOTOR1A = 1;
						MOTOR1B	= 0;

						MOTOR2A = 1;
						MOTOR2B	= 1;
						break;
					}
				case 'U':
					{	
						MOTOR1A = 1;
						MOTOR1B	= 0;

						MOTOR2A = 1;
						MOTOR2B	= 0;
						break;
					}
				case 'D':
					{	
						MOTOR1A = 0;
						MOTOR1B	= 1;

						MOTOR2A = 0;
						MOTOR2B	= 1;
						break;
					}
				default:
					{
						MOTOR1A = 1;
						MOTOR1B	= 1;

						MOTOR2A = 1;
						MOTOR2B	= 1;
						break;
					}
			}
			delay_ms(1000);
			MOTOR1A = 1;
			MOTOR1B	= 1;

			MOTOR2A = 1;
			MOTOR2B	= 1;
			// recv中断持续有
			recv_num = 0;
		}
	}
}


void Esp8266_init()
{
  send_string(CMD_MODE);
  delay_ms(3000);
  //check_ok();

  send_string(CMD_RST);
  delay_ms(5000);

  /*send_string(CMD_WIFI);*/

  recv_num = 0;
  send_string(CMD_MUX);
  //check_ok();
  delay_ms(3000);

  recv_num = 0;
  send_string(CMD_SERVER);
  //check_ok();
  delay_ms(3000);
}

 int main()
 {
 	unsigned char first = 1;
 	LED = 0;
    delay_ms(5000);
	Usart_init();
	Esp8266_init();

	while(1)
	{
		get_data();
		delay_ms(5);
	}
 	return 0;
}