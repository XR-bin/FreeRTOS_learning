//FreeRTOS相关文件
#include "FreeRTOS.h"
#include "task.h"
//我们的硬件相关文件
#include "stm32f10x.h"
#include "led.h"
#include "usart.h"

/*****************************************************************************
*																本章知识
*		1、认识xTaskCreate函数
*						xTaskCreate的参数：
*                      参数1：任务函数
*                      参数2：任务名
*                      参数3：任务堆栈大小
*                      参数4：传递给任务函数的参数
*                      参数5：任务优先级
*                      参数7：任务句柄
*          xTaskCreate的返回值：pdPASS(表示任务创建成功：1)或pdFALSE(表示任务创建失败：0)
*
*		2、动态创建任务的特点：
*            不需要自己定义任务堆栈(数组)，我们只需设置堆栈的大小就行，且自动分配的堆栈空间
*        在任务被删除后可以被释放。
*
*   3、使用动态创建任务时需注意：
*						1、要将在FreeRTOSConfig.h中configSUPPORT_DYNAMIC_ALLOCATION宏要置1
*           2、注意FreeRTOSConfig.h中configTOTAL_HEAP_SIZE的大小，它决定
*              了pvPortMalloc函数所能够支配的空间大小
*
*   4、静态创建和动态创建的区别在于定义任务堆栈空间的方式，静态创建方式的堆栈空间是不能被
*      释放的，而动态创建方式的任务堆栈空间是可以的，相比较动态创建的方式对空间把控比较灵
*      活，比较推荐使用动态方式创建任务。
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
static TaskHandle_t AppTaskCreate_Handle=NULL;
//创建任务AppTaskCreate的任务函数名
static void AppTaskCreate(void);


//LED任务句柄
static TaskHandle_t LED_Task_Handle=NULL;	
//LED任务的任务函数名
static void LED_Task(void* parameter);


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
	BaseType_t xReturn = pdPASS;//定义一个创建信息返回值 pdPASS：成功

	//进入临界区，创建任务过程我们必须保证在临界区
	taskENTER_CRITICAL();        
	
	//创建LED_Task任务
	xReturn = xTaskCreate(
								(TaskFunction_t	)LED_Task,		     //任务函数
								(const char* 	  )"LED_Task",		   //任务名称
								(uint32_t 		  )128,					     //任务堆栈大小
								(void* 		  	  )NULL,				     //传递给任务函数的参数
								(UBaseType_t 	  )1, 				       //任务优先级
								(TaskHandle_t*  )&LED_Task_Handle);//任务句柄 
																			
	if(xReturn == pdPASS) printf("LED_Task任务创建成功!\r\n");
	else                  printf("LED_Task任务创建失败!\r\n");
	
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
		LED1_OFF;
		vTaskDelay(500);   /* 延时500个tick */
		printf("LED0开\r\n");
		printf("LED1关\r\n");
	
		LED0_OFF; 
		LED1_ON;    
		vTaskDelay(500);   /* 延时500个tick */		 		
		printf("LED0关\r\n");
		printf("LED1开\r\n");
	}
}

