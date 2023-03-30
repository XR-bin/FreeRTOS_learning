#include "stm32f10x.h"

/*****************************************************
*函数功能  ：外部中断3/4初始化
*函数名    ：EXTI34_Init
*函数参数  ：void
*函数返回值：void
*描述      ：
*           低电平按下，高电平抬起
*						KEY0     PE4
*           KEY1     PE3
********************************************************/
void EXTI34_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;  //GPIOx配置结构体
	NVIC_InitTypeDef  NVIC_InitStruct;  //中断配置结构体
	EXTI_InitTypeDef  EXTI_InitStruct;  //外部中断线配置结构体
	
	//时钟使能     AFIO  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);   	//使能复用功能时钟	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);   //GPIOE
	
	//GPIOx配置
  //PE
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4;	      //PE3/4端口配置
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU ; 	          //上拉输入
  GPIO_Init(GPIOE, &GPIO_InitStruct);					            //根据设定参数初始化GPIE PE4 
	
	/*中断配置*/
	//外部中断3
	NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;			        //外部中断3
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x05;	//抢占优先级5
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;				//子优先级0 
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;							//使能外部中断通道
  NVIC_Init(&NVIC_InitStruct);  	                          //配置
	//外部中断4
	NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;			        //外部中断4
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x05;	//抢占优先级5
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;				//子优先级0 
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;							//使能外部中断通道
  NVIC_Init(&NVIC_InitStruct);  	                          //配置
	
	/*外部中断配置*/
	//外部中断配置寄存器清零
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource3);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource4);
	//开始配置外部中断线
	//外部中断3
	EXTI_InitStruct.EXTI_Line=EXTI_Line3;                     //选择外部中断线3
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;	        //中断事件
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;      //下降沿触发    (EXTI_RTSR寄存器的第0位)
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;                    //使能3线的中断 (EXTI->IMR寄存器的第0位)
  EXTI_Init(&EXTI_InitStruct);	  	                        //初始化外部中断3
	//外部中断4
  EXTI_InitStruct.EXTI_Line=EXTI_Line4;                     //选择外部中断线4
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;	        //中断事件
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;      //下降沿触发    (EXTI_RTSR寄存器的第0位)
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;                    //使能4线的中断 (EXTI->IMR寄存器的第0位)
  EXTI_Init(&EXTI_InitStruct);	  	                        //初始化外部中断4
}


