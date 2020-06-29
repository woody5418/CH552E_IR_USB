/********************************** (C) COPYRIGHT *******************************
* File Name          : timer.C
* Author             : WCH
* Version            : V1.1
* Date               : 2020/6/16
* Description        : CH552 Timer CAP1���벶��ʹ�ܺ��жϴ���
*******************************************************************************/

#include "..\Public\CH552.H"
#include "..\Public\Debug.H"
#include "timer.h"
#include "stdio.h"

#pragma  NOAREGS

UINT8  VS1838_Status = 0;         //������մ���״̬
UINT8  VS1838_Receive_Count = 0;  //�����������λ����
UINT8  VS1838_receive_ok = 0;     //���������ɱ�־
UINT32 VS838_Receive_Data = 0;    //32λ����
UINT8  VS1838_Data = 0;    		  //���ⰴ����ֵ
/*******************************************************************************
* Function Name  : mTimer_x_SetData(UINT8 x,UINT16 dat)
* Description    : CH554Timer0 TH0��TL0��ֵ
* Input          : UINT16 dat;��ʱ����ֵ
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
        RCAP2L = TL2 = tmp & 0xff;                                               //16λ�Զ����ض�ʱ��
        RCAP2H = TH2 = (tmp >> 8) & 0xff;
    }
}

/*******************************************************************************
* Function Name  : CAP1Init(UINT8 mode)
* Description    : CH554��ʱ������2 T2���Ų�׽���ܳ�ʼ��T2
                   UINT8 mode,���ز�׽ģʽѡ��
                   0:T2ex���½��ص���һ���½���
                   1:T2ex�������֮��
                   3:T2ex�������ص���һ��������
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
    T2MOD = T2MOD & ~T2OE | (mode << 2) | bT2_CAP1_EN;                         //ʹ��T2���Ų�׽����,���ز�׽ģʽѡ��
}
/*******************************************************************************
* Function Name  : IR_Init()
* Description    : ��ʼ��
*******************************************************************************/
void IR_Init(void)
{
    mTimer2Clk12DivFsys();       //T2��ʱ��ʱ������
    mTimer_x_SetData(2, 0);     //T2 ��ʱ��ģʽ���ò�׽ģʽ
    PIN_FUNC |= bT2_PIN_X;		//CAP1��P10 ӳ�䵽P14
    CAP1Init(0);			    //ʹ��T2���Ų�׽����,���ز�׽ģʽѡ�� 0:T2ex���½��ص���һ���½���
    mTimer2RunCTL(1);           //T2��ʱ������
    ET2 = 1;                    //T2��ʱ���жϿ���
}
/*******************************************************************************
* Function Name  : IR_Parsing()
* Description    : ����IRDATA  ����ֵ
*******************************************************************************/
void VS1838ReceiveHandle(UINT16 capdata)
{
    switch(VS1838_Status) {
    case 0: //������ 9ms + 4.5ms    13.5
        if((capdata >= 10000) && (capdata <= 16000)) {
            VS1838_Status = 1;
        } else {
            VS1838_Status = 0;
            VS1838_Receive_Count = 0;
        }
        break;
    case 1: //��ʼ32λ���ݵĽ���
        if((capdata >= 500) && (capdata <= 1600)) { //���1.12ms ->0
            VS838_Receive_Data = VS838_Receive_Data << 1;
            VS1838_Receive_Count++;
        } else if((capdata >= 1600) && (capdata <= 2700)) { //���2.25ms ->1
            VS838_Receive_Data = VS838_Receive_Data << 1;
            VS838_Receive_Data = VS838_Receive_Data | 0x0001;
            VS1838_Receive_Count++;
        } else {
            //printf("error capdata= %d\r\n", capdata);
            VS1838_Status = 0;
            VS838_Receive_Data = 0;
            VS1838_Receive_Count = 0;
        }
        //������������λ����������һ��
        while(VS1838_Receive_Count == 32) {
            //printf("%d\r\n", VS838_Receive_Data);
			//printf("%d\r\n", (UINT16)VS1838_Receive_Count);
            VS1838_receive_ok = 1; //����������
            VS1838_Status = 0;
            VS1838_Receive_Count = 0;
            break;
        }
        break;
    default : { //ȱʡ��
        VS1838_Status = 0;
    }
    break;
    }
}



//��ȡ����ֵ
UINT8 VS1838_Process(void)
{
    UINT8 Address_H, Address_L;      //��ַ��,��ַ����
    UINT8 Data_H, Data_L;            //������,���ݷ���

    if(VS1838_receive_ok == 1) {      //�������
        Address_H = VS838_Receive_Data >> 24;            //�õ���ַ��
        Address_L = (VS838_Receive_Data >> 16) & 0xff;   //�õ���ַ����
        if((Address_H == (UINT8)~Address_L)) { //����ң��ʶ����(ID)����ַ
            Data_H = VS838_Receive_Data >> 8;          //�õ�������
            Data_L = VS838_Receive_Data;               //�õ����ݷ���
            if(Data_H == (UINT8)~Data_L) {             //������������ȷ
                VS1838_Data = Data_H;                    //��ȷ��ֵ
                VS1838_Status = 0;
                VS838_Receive_Data = 0;
				//printf("0x%04x\r\n",(UINT16)VS1838_Data);
                return VS1838_Data;
            }
        }
    }
    return  0;
}



//�������ֵ ��һ��Ϊ��ֵ
static void Clean_countReg(void)
{
    RCAP2H = TH2 = 0XFF;
    RCAP2L = TL2 = 0XFF;
}

/*******************************************************************************
* Function Name  : mTimer2Interrupt()
* Description    : CH554��ʱ������0��ʱ�������жϴ�������
*******************************************************************************/
void mTimer2Interrupt(void) interrupt INT_NO_TMR2 using 3                //timer2�жϷ������,ʹ�üĴ�����3
{
    mTimer2RunCTL( 0 );             //�ض�ʱ��
    if(CAP1F) {                     //T2��ƽ��׽�жϱ�־
        VS1838ReceiveHandle(T2CAP1);
        Clean_countReg();//mTimer_x_SetData(2,0);
        CAP1F = 0;           		//���T2��׽�жϱ�־
    }
    if(TF2) TF2 = 0;         		//��ն�ʱ��2����ж�
    mTimer2RunCTL( 1 );      		//����ʱ��
}
