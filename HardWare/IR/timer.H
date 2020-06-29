#ifndef	__TIMER_H__
#define __TIMER_H__
 
#define mTimer2Clk12DivFsys( ) {T2MOD &= ~ bT2_CLK;C_T2 = 0;}      //定时器,时钟=Fsys/12 T2标准时钟
#define mTimer2ClkFsys( )      {T2MOD |= (bTMR_CLK | bT2_CLK);C_T2=0;}         //定时器,时钟=Fsys
#define mTimer2Clk4DivFsys( )  {T2MOD &= ~bTMR_CLK;T2MOD |=  bT2_CLK;C_T2 = 0;}//定时器,时钟=Fsys/4
#define mTimer2CountClk( )     {C_T2 = 1;}                                     //计数器,T2引脚的下降沿有效

//CH554 Timer2 开始(SS=1)/结束(SS=0)
#define mTimer2RunCTL( SS )    {TR2 = SS ? START : STOP;}
#define mTimer2OutCTL( )       (T2MOD |= T2OE)              //T2输出  频率TF2/2   
#define CAP1Alter( )           {PIN_FUNC |= bT2_PIN_X;}     //CAP1由P10 映射到P14
#define CAP2Alter( )           {PIN_FUNC |= bT2EX_PIN_X;}   //CAP2由P11 映射RST


void IR_Init(void);
UINT8 VS1838_Process(void);

#endif