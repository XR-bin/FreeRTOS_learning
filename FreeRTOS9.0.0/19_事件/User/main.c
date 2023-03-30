//FreeRTOS相关文件
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
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


//LED0任务句柄
static TaskHandle_t LED0_Task_Handle=NULL;	
//LED0任务的任务函数名
static void LED0_Task(void* parameter);

//LED1任务句柄
static TaskHandle_t LED1_Task_Handle=NULL;	
//LED1任务的任务函数名
static void LED1_Task(void* parameter);

//KEY任务句柄
static TaskHandle_t KEY_Task_Handle=NULL;	
//KEY任务的任务函数
static void Key_Task(void* parameter);

//事件句柄
static EventGroupHandle_t Event_Handle =NULL;
//事件位可以想象成一个变量的某个位，当某个位置1，表示发生某个事件
//如果事件设置的是32位的，只有24位是事件位
//如果事件设置的是16位的，只有16位是事件位
//注意根据宏修改的，决定事件是32位的还是16位的
//感兴趣的位
#define KEY1_EVENT  (0x01 << 0)//设置事件1掩码的位0
#define KEY2_EVENT  (0x01 << 1)//设置事件2掩码的位1
#define KEY3_EVENT  (0x01 << 2)//设置事件3掩码的位2
#define KEY4_EVENT  (0x01 << 3)//设置事件4掩码的位3

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

	//创建事件
	Event_Handle = xEventGroupCreate();	 
  if(NULL != Event_Handle) printf("Event_Handle 事件创建成功!\r\n");
	
	//创建LED0_Task任务
	xReturn = xTaskCreate(
								(TaskFunction_t	)LED0_Task,		      //任务函数
								(const char* 	  )"LED0_Task",		    //任务名称
								(uint32_t 		  )128,					      //任务堆栈大小
								(void* 		  	  )NULL,				      //传递给任务函数的参数
								(UBaseType_t 	  )1, 				        //任务优先级
								(TaskHandle_t*  )&LED0_Task_Handle);//任务句柄 
								
	if(xReturn == pdPASS) printf("LED0_Task任务创建成功!\r\n");
	
	//创建LED1_Task任务
	xReturn = xTaskCreate(
								(TaskFunction_t	)LED1_Task,		      //任务函数
								(const char* 	  )"LED1_Task",		    //任务名称
								(uint32_t 		  )128,					      //任务堆栈大小
								(void* 		  	  )NULL,				      //传递给任务函数的参数
								(UBaseType_t 	  )2, 				        //任务优先级
								(TaskHandle_t*  )&LED1_Task_Handle);//任务句柄 
								
	if(xReturn == pdPASS) printf("LED1_Task任务创建成功!\r\n");
	
	//创建Key_Task任务
	xReturn = xTaskCreate(
								(TaskFunction_t	)Key_Task,		     //任务函数
								(const char* 	  )"Key_Task",		   //任务名称
								(uint32_t 		  )128,					     //任务堆栈大小
								(void* 		  	  )NULL,				     //传递给任务函数的参数
								(UBaseType_t 	  )3, 				       //任务优先级
								(TaskHandle_t*  )&KEY_Task_Handle);//任务句柄 
																			
	if(xReturn == pdPASS) printf("Key_Task任务创建成功!\r\n");
	
	//删除创建任务AppTaskCreate的任务(就是当前这个函数自己)
	vTaskDelete(AppTaskCreate_Handle);
	//退出临界区
	taskEXIT_CRITICAL();            
}

/*********************************************************************
*函数功能  ：LED0灯相关的操作
*函数名    ：LED0_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void LED0_Task(void* parameter)
{
	EventBits_t r_event;  //定义一个事件接收变量
	while (1)
	{
		//等待事件发生
		r_event = xEventGroupWaitBits(Event_Handle,         //事件对象句柄
                                  KEY1_EVENT|KEY2_EVENT,//接收线程感兴趣的事件
                                  pdTRUE,               //退出时清除事件位
                                  pdTRUE,               //满足感兴趣的所有事件(逻辑与)
                                  portMAX_DELAY);       //指定超时事件,一直等
																	
		if((r_event & (KEY1_EVENT|KEY2_EVENT)) == (KEY1_EVENT|KEY2_EVENT)) 
    {
      printf ( "KEY1与KEY2都按下\r\n");		
      LED0_TOGGLE;       //LED0	反转
    }
    else
      printf ( "事件错误！\r\n");	
		
		
	}
}


/*********************************************************************
*函数功能  ：LED1灯相关的操作
*函数名    ：LED1_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void LED1_Task(void* parameter)
{
	EventBits_t r_event;  //定义一个事件接收变量
	while (1)
	{
		//等待事件发生
		r_event = xEventGroupWaitBits(Event_Handle,         //事件对象句柄
                                  KEY3_EVENT|KEY4_EVENT,//接收线程感兴趣的事件
                                  pdTRUE,               //退出时清除事件位
                                  pdFALSE,              //满足感兴趣的其中一个事件(逻辑或)
                                  portMAX_DELAY);       //指定超时事件,一直等
																	
		if(r_event & (KEY3_EVENT|KEY4_EVENT))
    {
      printf ( "KEY1或KEY2都按下\r\n");		
      LED1_TOGGLE;       //LED1	反转
    }
    else
      printf ( "事件错误！\r\n");	
			
		vTaskDelay(30);
	}
}


/*********************************************************************
*函数功能  ：按键相关的操作
*函数名    ：Key_Task
*函数参数  ：void* parameter
*函数返回值：static void
*描述      ：
**********************************************************************/
static void Key_Task(void* parameter)
{
	while (1)
	{
		switch(KEY_Scan())
		{
			case 1: 
				printf ( "KEY1被按下,触发事件1和事件3\r\n" );
				//触发事件1
				xEventGroupSetBits(Event_Handle,KEY1_EVENT);
				xEventGroupSetBits(Event_Handle,KEY3_EVENT);
				break;
			case 2: 
				printf ( "KEY2被按下，触发事件2和事件4\r\n" );	
				//触发事件2
				xEventGroupSetBits(Event_Handle,KEY2_EVENT); 	
				xEventGroupSetBits(Event_Handle,KEY4_EVENT);
				break;
			default: break;
		}
		
		vTaskDelay(30);
	}
}




