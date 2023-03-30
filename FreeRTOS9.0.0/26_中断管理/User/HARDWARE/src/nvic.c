#include "stm32f10x.h" 
#include "stdio.h"
//FreeRTOS相关文件
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static void delay_ms(u8 ms)
{
	u32 i;
	for(i=0; i< ms*100; i++);
}

/******************************************外部中断******************************************/
/* 声明引用外部队列 */
extern QueueHandle_t Test_Queue;
static uint32_t send_data1 = 1;
static uint32_t send_data2 = 2;
/******************************************************************
*函数功能  ：外部中断3服务函数
*函数名    ：EXTI3_IRQHandler
*函数参数  ：void
*函数返回值：void
*描述      ：
*******************************************************************/
void EXTI3_IRQHandler(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
	//确保是否产生了EXTI Line中断
  uint32_t ulReturn;
  //进入临界段，临界段可以嵌套
  ulReturn = taskENTER_CRITICAL_FROM_ISR();
	//判断外部中断线3是否真的触发   SET=1   RESET=0
  if(EXTI_GetFlagStatus(EXTI_Line3) == SET)
  {
    delay_ms(60);
		//滤波
    if(!(GPIOE->IDR & (1<<3)))
    {
			// 将数据写入（发送）到队列中，等待时间为 0 
			xQueueSendFromISR(Test_Queue, //消息队列的句柄
												&send_data1,//发送的消息内容 
												&pxHigherPriorityTaskWoken);
			
			//如果需要的话进行一次任务切换
			portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    }
  }
	 EXTI->PR |= (1<<3);
	//退出临界段
  taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}

/******************************************************************
*函数功能  ：外部中断4服务函数
*函数名    ：EXTI4_IRQHandler
*函数参数  ：void
*函数返回值：void
*描述      ：
*******************************************************************/
void EXTI4_IRQHandler(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
  //确保是否产生了EXTI Line中断
  uint32_t ulReturn;
  //进入临界段，临界段可以嵌套
	ulReturn = taskENTER_CRITICAL_FROM_ISR();

	//判断外部中断线4是否真的触发   SET=1   RESET=0
  if(EXTI_GetFlagStatus(EXTI_Line4) == SET)
  {
    delay_ms(60);
		//滤波
    if(!(GPIOE->IDR & (1<<4)))
    {
			// 将数据写入（发送）到队列中，等待时间为 0 
			xQueueSendFromISR(Test_Queue, //消息队列的句柄
												&send_data2,//发送的消息内容 
												&pxHigherPriorityTaskWoken);
			
			//如果需要的话进行一次任务切换
			portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    }
  }
	
	EXTI->PR |= (1<<4);
	//退出临界段
  taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}





