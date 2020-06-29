
/********************************** (C) COPYRIGHT *******************************
* File Name          : Main.C
* Author             : WCH
* Version            : V1.2
* Date               : 2020/6/15
* Description        : 
					   
 ////////////////////////////  < 重点问题 >  ////////////////////////////
 
 ****unsigned char 类型的变量无法使用printf 打印 ****
 
 ****使用UART1的时候 需要重定向purchar  int putchar(int c) ****
 
*******************************************************************************/
#include "includes.h"

#pragma  NOAREGS


void main() 
{
	CfgFsys();			//Freq = 12MHz. 
    mDelaymS(5);        //等待系统稳定
	UART1Setup();		//串口1初始化 波特率 57600 bps. 	
	USBDeviceInit();    //USB设备模式初始化
	IR_Init();
	EA = 1;				// 启用全局中断. 
	UEP1_T_LEN = 0;     //预使用发送长度一定要清空
    UEP2_T_LEN = 0;                     
	//预使用发送长度一定要清空
	
	FLAG = 0;
    Ready = 0;
    while ( 1 )
    {
		if(Ready) {
			HIDValueHandle();
		}
	}
}
