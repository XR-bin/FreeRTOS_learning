//FreeRTOS相关文件
#include "FreeRTOS.h"
#include "task.h"
//我们的硬件相关文件
#include "stm32f10x.h"
#include "usart.h"
#include "led.h"
#include "time.h"
#include "string.h"

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

//LED0任务任务句柄
static TaskHandle_t LED0_Task_Handle = NULL;
//LED0任务的任务函数
static void LED0_Task(void* parameter);

//LED1任务任务句柄
static TaskHandle_t LED1_Task_Handle = NULL;
//LED1任务的任务函数
static void LED1_Task(void* parameter);

//CPU利用率查询任务句柄
static TaskHandle_t CPU_Task_Handle = NULL;
//CPU利用率查询的任务函数
static void CPU_Task(void* parameter);

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
	LED_Init();
	BASIC_TIM_Init();
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
	//创建LED0_Task任务
	xReturn = xTaskCreate(
								(TaskFunction_t	)LED0_Task,		      //任务函数
								(const char* 	  )"LED0_Task",		    //任务名称
								(uint32_t 		  )128,					      //任务堆栈大小
								(void* 		  	  )NULL,				      //传递给任务函数的参数
								(UBaseType_t 	  )1, 				        //任务优先级
								(TaskHandle_t*  )&LED0_Task_Handle); //任务句柄 
																			
	if(xReturn == pdPASS) printf("LED0_Task任务创建成功!\r\n");
	
	//创建LED1_Task任务
	xReturn = xTaskCreate(
								(TaskFunction_t	)LED1_Task,		      //任务函数
								(const char* 	  )"LED1_Task",		    //任务名称
								(uint32_t 		  )128,					      //任务堆栈大小
								(void* 		  	  )NULL,				      //传递给任务函数的参数
								(UBaseType_t 	  )2, 				        //任务优先级
								(TaskHandle_t*  )&LED1_Task_Handle); //任务句柄 
																			
	if(xReturn == pdPASS) printf("LED1_Task任务创建成功!\r\n");
	
	//创建CPU_Task任务
	xReturn = xTaskCreate(
								(TaskFunction_t	)CPU_Task,		      //任务函数
								(const char* 	  )"CPU_Task",		    //任务名称
								(uint32_t 		  )512,					      //任务堆栈大小
								(void* 		  	  )NULL,				      //传递给任务函数的参数
								(UBaseType_t 	  )3, 				        //任务优先级
								(TaskHandle_t*  )&CPU_Task_Handle);//任务句柄 
																			
	if(xReturn == pdPASS) printf("CPU_Task任务创建成功!\r\n");
	
	//删除创建任务AppTaskCreate的任务(就是当前这个函数自己)
	vTaskDelete(AppTaskCreate_Handle);
	//退出临界区
	taskEXIT_CRITICAL();            
}

/*********************************************************************
*函数功能  ：LED0相关的操作
*函数名    ：LED0_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void LED0_Task(void* parameter)
{
	while(1)
	{
		LED0_ON;
		printf("LED0_Task Running,LED0_ON\r\n");
		vTaskDelay(500);
		LED0_OFF;
		printf("LED0_Task Running,LED0_OFF\r\n");
		vTaskDelay(500);
	}
}

/*********************************************************************
*函数功能  ：LED1相关的操作
*函数名    ：LED1_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void LED1_Task(void* parameter)
{
	while(1)
	{
		LED1_ON;
		printf("LED1_Task Running,LED0_ON\r\n");
		vTaskDelay(500);
		LED1_OFF;
		printf("LED1_Task Running,LED0_OFF\r\n");
		vTaskDelay(500);
	}
}

/*********************************************************************
*函数功能  ：CPU利用率相关的操作
*函数名    ：CPU_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void CPU_Task(void* parameter)
{	
	uint8_t CPU_RunInfo[400];		//保存任务运行时间信息
	
  while (1)
  {
		memset(CPU_RunInfo,0,400);				//信息缓冲区清零
    
    vTaskList((char *)&CPU_RunInfo);  //获取任务运行时间信息
    
    printf("---------------------------------------------\r\n");
    printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n");
    
    memset(CPU_RunInfo,0,400);				//信息缓冲区清零
    
    vTaskGetRunTimeStats((char *)&CPU_RunInfo);
    
    printf("任务名       运行计数         利用率\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n\n");
    vTaskDelay(1000);   //延时1000个tick	
	}
	
}

