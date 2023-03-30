#include "stm32f10x_it.h"
#include "stm32f10x.h"
#include "usart.h"
#include "FreeRTOS.h"					//FreeRTOS使用		  
#include "task.h" 

/* 用于统计运行时间 */
volatile uint32_t CPU_RunTime = 0UL;

/******************************************定时器中断******************************************/
/******************************************
*函数功能  ：定时器6的中断服务函数
*函数名    ：TIM6_DAC_IRQHandler ：
*函数参数  ：无
*函数返回值：无
*函数描述  ：
*********************************************/
void TIM6_IRQHandler(void)
{
	//检查TIM6更新中断发生与否
  if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
  { 
    CPU_RunTime++;
  }
	//清除TIMx更新中断标志 
	TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
}


