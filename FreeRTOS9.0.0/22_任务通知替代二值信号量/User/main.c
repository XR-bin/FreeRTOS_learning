//FreeRTOS相关文件
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "limits.h"
//我们的硬件相关文件
#include "stm32f10x.h"
#include "key.h"
#include "usart.h"
#include "led.h"

/*****************************************************************************************
*																       本章知识
*		1、需要了解的函数：
*		
*                 
******************************************************************************************/

/****************************************************************************** 
*                             什么是任务句柄
* 知识补充：
* 			任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
* 	以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
* 	这个句柄可以为NULL。
*
*   说直白点，任务句柄就是一个指向任务控制块的指针，任务控制块存着任务的所有信息
*******************************************************************************/


/*****************************************
*注意：
*    FreeRTOS推荐的最小任务栈为512个字节
*    创建两个128字大小的任务栈
*    1个字等于4个字节
*    FreeRTOS推荐的最小任务栈为512个字节
******************************************/

//创建任务AppTaskCreate的任务句柄
static TaskHandle_t AppTaskCreate_Handle=NULL;
//创建任务AppTaskCreate的任务函数名
static void AppTaskCreate(void);

//接收任务通知1的任务句柄
static TaskHandle_t Receive1_Task_Handle = NULL;
//接收任务通知1的任务函数
static void Receive1_Task(void* pvParameters);

//接收任务通知2的任务句柄
static TaskHandle_t Receive2_Task_Handle = NULL;
//接收任务通知2的任务函数
static void Receive2_Task(void* pvParameters);

//发送任务通知的任务句柄
static TaskHandle_t Send_Task_Handle = NULL;
//发送任务通知的任务函数
static void Send_Task(void* pvParameters);/* Send_Task任务实现 */


static void BSP_Init(void);

int main(void)
{
	BaseType_t xReturn = pdPASS;//定义一个创建信息返回值 pdPASS：成功
	
	//硬件初始化
	BSP_Init();    
	printf("硬件初始化完成\r\n");
	
	//动态创建方式创建任务
	//创建任务的操作我们把它交给AppTaskCreate任务来操作，这样方便管理
	xReturn = xTaskCreate(
									(TaskFunction_t	)AppTaskCreate,		      //任务函数
									(const char* 	  )"AppTaskCreate",		    //任务名称
									(uint16_t 		  )128,	                  //任务堆栈大小
									(void* 		  	  )NULL,				          //传递给任务函数的参数
									(UBaseType_t 	  )1, 	                  //任务优先级
									(TaskHandle_t*  )&AppTaskCreate_Handle);//任务句柄  
															
															 
	
	//创建成功就可以开启任务调度器了
	if(xReturn == pdPASS)  vTaskStartScheduler();   /* 启动任务，开启调度 */
	else                   printf("任务创建失败\r\n");
		
	while(1);
}


/*********************************************************************
*函数功能  ：硬件初始化
*函数名    ：BSP_Init
*函数参数  ：void
*函数返回值：static void
*描述      ：
*            所有硬件初始化都写在该函数里。
*            例如：串口初始化、定时器初始化、LED初始、蜂鸣器初始化.....
**********************************************************************/
static void BSP_Init(void)
{
	/*
	* STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	* 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	* 都统一用这个优先级分组，千万不要再分组，切忌。
	*/
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	USART1_Init(115200);
	KEY_Init();
	LED_Init();
}



/*****************************************************
*函数功能  ：专门创建任务的任务
*函数名    ：AppTaskCreate
*函数参数  ：void
*函数返回值：static void
*描述      ：
*            这个任务创建函数是专门用来创建任务函数的，我
*        们会把它当任务创建，当它把其他任务创建完成后，我
*        们会我们会把该任务销毁。
********************************************************/
static void AppTaskCreate(void)
{
	BaseType_t xReturn = pdPASS;//定义一个创建信息返回值 pdPASS：成功
	
	//进入临界区，创建任务过程我们必须保证在临界区
	taskENTER_CRITICAL();   
	
	//创建Receive1_Task任务
	xReturn = xTaskCreate(
												(TaskFunction_t	)Receive1_Task,		      //任务函数
												(const char* 	  )"Receive1_Task",		    //任务名称
												(uint16_t 		  )128,	                  //任务堆栈大小
												(void* 		  	  )NULL,				          //传递给任务函数的参数
												(UBaseType_t 	  )1, 	                  //任务优先级
												(TaskHandle_t*  )&Receive1_Task_Handle);//任务句柄  
	if(pdPASS == xReturn) printf("创建Receive1_Task任务成功!\r\n");
	
	//创建Receive2_Task任务
	xReturn = xTaskCreate(
												(TaskFunction_t	)Receive2_Task,		      //任务函数
												(const char* 	  )"Receive2_Task",		    //任务名称
												(uint16_t 		  )128,	                  //任务堆栈大小
												(void* 		  	  )NULL,				          //传递给任务函数的参数
												(UBaseType_t 	  )2, 	                  //任务优先级
												(TaskHandle_t*  )&Receive2_Task_Handle);//任务句柄  
	if(pdPASS == xReturn) printf("创建Receive2_Task任务成功!\r\n");
	
	//创建Send_Task任务
	xReturn = xTaskCreate(
												(TaskFunction_t	)Send_Task,		      //任务函数
												(const char* 	  )"Send_Task",		    //任务名称
												(uint16_t 		  )128,	              //任务堆栈大小
												(void* 		  	  )NULL,				      //传递给任务函数的参数
												(UBaseType_t 	  )3, 	              //任务优先级
												(TaskHandle_t*  )&Send_Task_Handle);//任务句柄  
	if(pdPASS == xReturn) printf("创建Send_Task任务成功!\r\n");
	
	//删除创建任务AppTaskCreate的任务(就是当前这个函数自己)
	vTaskDelete(AppTaskCreate_Handle);
	//退出临界区
	taskEXIT_CRITICAL();            
}

/*********************************************************************
*函数功能  ：接收任务通知1相关的操作
*函数名    ：Receive1_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void Receive1_Task(void* parameter)
{
  while(1)
	{
		//等待任务通知，没消息一直等
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
    printf("Receive1_Task 任务通知获取成功!\r\n");                      
		LED0_TOGGLE;   //LED翻转
	}
}


/*********************************************************************
*函数功能  ：接收任务通知2相关的操作
*函数名    ：Receive2_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void Receive2_Task(void* parameter)
{
  while(1)
	{
		//等待任务通知，没消息一直等
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
    printf("Receive2_Task 任务通知获取成功!\r\n"); 
		
		LED1_TOGGLE;   //LED翻转
	}
}


/*********************************************************************
*函数功能  ：发送任务通知相关的操作
*函数名    ：Send_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void Send_Task(void* parameter)
{
	BaseType_t xReturn = pdTRUE;    //定义一个创建信息返回值，pdTRUE：成功

  while(1)
	{
		switch(KEY_Scan())
		{
			case 1: 
				xReturn = xTaskNotifyGive(Receive1_Task_Handle);
				if( xReturn == pdPASS ) printf("Receive1_Task_Handle 任务通知消息发送成功!\r\n");
				break;
				
			case 2: 
				xReturn = xTaskNotifyGive(Receive2_Task_Handle);
				if( xReturn == pdPASS ) printf("Receive2_Task_Handle 任务通知消息发送成功!\r\n");
				break;
			default: break;
		}
		
		vTaskDelay(30);
	}
}

