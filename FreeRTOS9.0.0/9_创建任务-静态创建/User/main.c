//FreeRTOS相关文件
#include "FreeRTOS.h"
#include "task.h"
//我们的硬件相关文件
#include "stm32f10x.h"
#include "led.h"
#include "usart.h"

/*****************************************************************************
*																本章知识
*		1、认识xTaskCreateStatic函数
*						xTaskCreateStatic的参数：
*                        参数1：任务函数
*                        参数2：任务名
*                        参数3：任务堆栈大小
*                        参数4：传递给任务函数的参数
*                        参数5：任务优先级
*                        参数6：任务堆栈
*                        参数7：任务控制块
*          xTaskCreateStatic的返回值：任务句柄
*
*		2、静态创建任务的特点：
*            需要自己定义任务堆栈(数组)，且我们自己定义的任务堆栈，在任务
*        被删除后它依然存在，我们是没办法去释放的
*
*   3、使用静态创建任务时需注意：
*						1、要将在FreeRTOSConfig.h中configSUPPORT_STATIC_ALLOCATION宏要置1
*           2、我们需要实现两个函数：vApplicationGetIdleTaskMemory()与 vApplicationGetTimerTaskMemory()，
*              这两个函数是用户设定的空闲（Idle）任务与定时器（Timer）任
*              务的堆栈大小，必须由用户自己分配，而不能是动态分配。
*
******************************************************************************/



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
static TaskHandle_t AppTaskCreate_Handle;
//创建任务AppTaskCreate的任务函数名
static void AppTaskCreate(void);
//创建任务AppTaskCreate的任务堆栈
static StackType_t AppTaskCreate_Stack[128];
//创建任务AppTaskCreate的任务控制块
static StaticTask_t AppTaskCreate_TCB;


//LED任务句柄
static TaskHandle_t LED_Task_Handle;	
//LED任务的任务函数名
static void LED_Task(void* parameter);
//LED任务的的任务堆栈
static StackType_t LED_Task_Stack[128];
//LED任务的的任务控制块
static StaticTask_t LED_Task_TCB;


static void BSP_Init(void);

int main(void)
{
	//硬件初始化
	BSP_Init();    
	printf("硬件初始化完成\r\n");
	
	//静态创建方式创建任务
	//创建任务的操作我们把它交给AppTaskCreate任务来操作，这样方便管理
	AppTaskCreate_Handle = xTaskCreateStatic(
															(TaskFunction_t	)AppTaskCreate,		   //任务函数
															(const char* 	  )"AppTaskCreate",		 //任务名称
															(uint32_t 		  )128,	               //任务堆栈大小
															(void* 		  	  )NULL,				       //传递给任务函数的参数
															(UBaseType_t 	  )3, 	               //任务优先级
															(StackType_t*   )AppTaskCreate_Stack,//任务堆栈
															(StaticTask_t*  )&AppTaskCreate_TCB);//任务控制块   
	
	//创建成功就可以开启任务调度器了
	if(NULL != AppTaskCreate_Handle)  vTaskStartScheduler();   /* 启动任务，开启调度 */
		
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
	LED_Init();
}



/*****************************************************
*函数功能  ：任务创建的任务
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
	//进入临界区，创建任务过程我们必须保证在临界区
	taskENTER_CRITICAL();        
	
	//创建LED_Task任务
	LED_Task_Handle = xTaskCreateStatic((TaskFunction_t	)LED_Task,		  //任务函数
																			(const char* 	  )"LED_Task",		//任务名称
																			(uint32_t 		  )128,					  //任务堆栈大小
																			(void* 		  	  )NULL,				  //传递给任务函数的参数
																			(UBaseType_t 	  )4, 				    //任务优先级
																			(StackType_t*   )LED_Task_Stack,//任务堆栈
																			(StaticTask_t*  )&LED_Task_TCB);//任务控制块 
																			
	if(NULL != LED_Task_Handle) printf("LED_Task任务创建成功!\r\n");
	else                        printf("LED_Task任务创建失败!\r\n");
	
	//删除创建任务AppTaskCreate的任务(就是当前这个函数自己)
	vTaskDelete(AppTaskCreate_Handle);
	//退出临界区
	taskEXIT_CRITICAL();            
}

/*********************************************************************
*函数功能  ：LED相关的操作
*函数名    ：LED_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void LED_Task(void* parameter)
{
	while (1)
	{
		LED0_ON;
		LED1_ON
		vTaskDelay(500);   /* 延时500个tick */
		printf("LED开\r\n");
	
		LED0_OFF
		LED1_OFF;     
		vTaskDelay(500);   /* 延时500个tick */		 		
		printf("LED关\r\n");
	}
}

/**************************************************************
*
*
*
*		注意：
*
*				当我们使用了静态创建任务的时候，在FreeRTOSConfig.h中
*   configSUPPORT_STATIC_ALLOCATION宏要置1，并且我们需要实现
*   两个函数：vApplicationGetIdleTaskMemory()与 vApplicationGetTimerTaskMemory()，
*   这两个函数是用户设定的空闲（Idle）任务与定时器（Timer）任
*   务的堆栈大小，必须由用户自己分配，而不能是动态分配。
*
*
*
*
****************************************************************/
// 空闲任务控制块
static StaticTask_t Idle_Task_TCB;	
// 空闲任务任务堆栈
static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];

// 定时器任务控制块
static StaticTask_t Timer_Task_TCB;
// 定时器任务堆栈
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];

/*********************************************************************
*函数功能  ：获取空闲任务的任务堆栈和任务控制块内存
*函数名    ：vApplicationGetIdleTaskMemory
*函数参数  ：ppxIdleTaskTCBBuffer   	:		任务控制块内存
*					   ppxIdleTaskStackBuffer	:	  任务堆栈内存
*					   pulIdleTaskStackSize	  :		任务堆栈大小
*函数返回值：void
*描述      ：
**********************************************************************/
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
																	 StackType_t **ppxIdleTaskStackBuffer, 
																	 uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer   = &Idle_Task_TCB;/* 任务控制块内存 */
	*ppxIdleTaskStackBuffer = Idle_Task_Stack;/* 任务堆栈内存 */
	*pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;/* 任务堆栈大小 */
}

/*********************************************************************
*函数功能  ：获取定时器任务的任务堆栈和任务控制块内存
*函数名    ：vApplicationGetTimerTaskMemory
*函数参数  ：ppxTimerTaskTCBBuffer   	:		任务控制块内存
*					   ppxTimerTaskStackBuffer	:	  任务堆栈内存
*					   pulTimerTaskStackSize	  :		任务堆栈大小
*函数返回值：void
*描述      ：
**********************************************************************/
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
																		StackType_t **ppxTimerTaskStackBuffer, 
																		uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer   = &Timer_Task_TCB;/* 任务控制块内存 */
	*ppxTimerTaskStackBuffer = Timer_Task_Stack;/* 任务堆栈内存 */
	*pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;/* 任务堆栈大小 */
}


