/********************************** (C) COPYRIGHT *******************************
* File Name          : timer.C
* Author             : WCH
* Version            : V1.1
* Date               : 2020/6/16
* Description        : CH552 Timer CAP1输入捕获使能和中断处理
*******************************************************************************/

#include "..\Public\CH552.H"
#include "..\Public\Debug.H"
#include "timer.h"
#include "stdio.h"

#pragma  NOAREGS

UINT8  VS1838_Status = 0;         //红外接收处理状态
UINT8  VS1838_Receive_Count = 0;  //红外接收数据位计数
UINT8  VS1838_receive_ok = 0;     //红外接收完成标志
UINT32 VS838_Receive_Data = 0;    //32位数据
UINT8  VS1838_Data = 0;    		  //红外按键的值
/*******************************************************************************
* Function Name  : mTimer_x_SetData(UINT8 x,UINT16 dat)
* Description    : CH554Timer0 TH0和TL0赋值
* Input          : UINT16 dat;定时器赋值
* Output         : None
* Return         : None
*******************************************************************************/
void mTimer_x_SetData(UINT8 x, UINT16 dat)
{
    UINT16 tmp;
    tmp = 65536 - dat;
    if(x == 0) {
        TL0 = tmp & 0xff;
        TH0 = (tmp >> 8) & 0xff;
    } else if(x == 1) {
        TL1 = tmp & 0xff;
        TH1 = (tmp >> 8) & 0xff;
    } else if(x == 2) {
        RCAP2L = TL2 = tmp & 0xff;                                               //16位自动重载定时器
        RCAP2H = TH2 = (tmp >> 8) & 0xff;
    }
}

/*******************************************************************************
* Function Name  : CAP1Init(UINT8 mode)
* Description    : CH554定时计数器2 T2引脚捕捉功能初始化T2
                   UINT8 mode,边沿捕捉模式选择
                   0:T2ex从下降沿到下一个下降沿
                   1:T2ex任意边沿之间
                   3:T2ex从上升沿到下一个上升沿
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAP1Init(UINT8 mode)
{
    RCLK = 0;
    TCLK = 0;
    CP_RL2 = 1;
    C_T2 = 0;
    T2MOD = T2MOD & ~T2OE | (mode << 2) | bT2_CAP1_EN;                         //使能T2引脚捕捉功能,边沿捕捉模式选择
}
/*******************************************************************************
* Function Name  : IR_Init()
* Description    : 初始化
*******************************************************************************/
void IR_Init(void)
{
    mTimer2Clk12DivFsys();       //T2定时器时钟设置
    mTimer_x_SetData(2, 0);     //T2 定时器模式设置捕捉模式
    PIN_FUNC |= bT2_PIN_X;		//CAP1由P10 映射到P14
    CAP1Init(0);			    //使能T2引脚捕捉功能,边沿捕捉模式选择 0:T2ex从下降沿到下一个下降沿
    mTimer2RunCTL(1);           //T2定时器启动
    ET2 = 1;                    //T2定时器中断开启
}
/*******************************************************************************
* Function Name  : IR_Parsing()
* Description    : 解析IRDATA  捕获值
*******************************************************************************/
void VS1838ReceiveHandle(UINT16 capdata)
{
    switch(VS1838_Status) {
    case 0: //引导码 9ms + 4.5ms    13.5
        if((capdata >= 10000) && (capdata <= 16000)) {
            VS1838_Status = 1;
        } else {
            VS1838_Status = 0;
            VS1838_Receive_Count = 0;
        }
        break;
    case 1: //开始32位数据的接收
        if((capdata >= 500) && (capdata <= 1600)) { //间隔1.12ms ->0
            VS838_Receive_Data = VS838_Receive_Data << 1;
            VS1838_Receive_Count++;
        } else if((capdata >= 1600) && (capdata <= 2700)) { //间隔2.25ms ->1
            VS838_Receive_Data = VS838_Receive_Data << 1;
            VS838_Receive_Data = VS838_Receive_Data | 0x0001;
            VS1838_Receive_Count++;
        } else {
            //printf("error capdata= %d\r\n", capdata);
            VS1838_Status = 0;
            VS838_Receive_Data = 0;
            VS1838_Receive_Count = 0;
        }
        //超出接收数据位数，接收下一个
        while(VS1838_Receive_Count == 32) {
            //printf("%d\r\n", VS838_Receive_Data);
			//printf("%d\r\n", (UINT16)VS1838_Receive_Count);
            VS1838_receive_ok = 1; //红外接收完成
            VS1838_Status = 0;
            VS1838_Receive_Count = 0;
            break;
        }
        break;
    default : { //缺省项
        VS1838_Status = 0;
    }
    break;
    }
}



//获取返回值
UINT8 VS1838_Process(void)
{
    UINT8 Address_H, Address_L;      //地址码,地址反码
    UINT8 Data_H, Data_L;            //数据码,数据反码

    if(VS1838_receive_ok == 1) {      //接收完成
        Address_H = VS838_Receive_Data >> 24;            //得到地址码
        Address_L = (VS838_Receive_Data >> 16) & 0xff;   //得到地址反码
        if((Address_H == (UINT8)~Address_L)) { //检验遥控识别码(ID)及地址
            Data_H = VS838_Receive_Data >> 8;          //得到数据码
            Data_L = VS838_Receive_Data;               //得到数据反码
            if(Data_H == (UINT8)~Data_L) {             //接收数据码正确
                VS1838_Data = Data_H;                    //正确键值
                VS1838_Status = 0;
                VS838_Receive_Data = 0;
				//printf("0x%04x\r\n",(UINT16)VS1838_Data);
                return VS1838_Data;
            }
        }
    }
    return  0;
}



//清除计数值 第一个为误值
static void Clean_countReg(void)
{
    RCAP2H = TH2 = 0XFF;
    RCAP2L = TL2 = 0XFF;
}

/*******************************************************************************
* Function Name  : mTimer2Interrupt()
* Description    : CH554定时计数器0定时计数器中断处理函数
*******************************************************************************/
void mTimer2Interrupt(void) interrupt INT_NO_TMR2 using 3                //timer2中断服务程序,使用寄存器组3
{
    mTimer2RunCTL( 0 );             //关定时器
    if(CAP1F) {                     //T2电平捕捉中断标志
        VS1838ReceiveHandle(T2CAP1);
        Clean_countReg();//mTimer_x_SetData(2,0);
        CAP1F = 0;           		//清空T2捕捉中断标志
    }
    if(TF2) TF2 = 0;         		//清空定时器2溢出中断
    mTimer2RunCTL( 1 );      		//开定时器
}

