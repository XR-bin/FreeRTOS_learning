//FreeRTOS相关文件
#include "FreeRTOS.h"
#include "task.h"
//我们的硬件相关文件
#include "stm32f10x.h"
#include "led.h"
#include "key.h"
#include "usart.h"

/*****************************************************************************************
*																       本章知识
*		1、任务的基本概念
*            从系统的角度看，任务是竞争系统资源的最小运行单元。 FreeRTOS 是一个支持多任务
*        的操作系统。在 FreeRTOS 中，任务可以使用或等待 CPU、使用内存空间等系统资源，并独
*        立于其它任务运行任何数量的任务可以共享同一个优先级，如果宏configUSE_TIME_SLICING
*        定义为 1， 处于就绪态的多个相同优先级任务将会以时间片切换的方式共享处理器。简单讲
*        就是在任何时刻，只有一个任务能得到运行，由FreeRTOS调度器决定运行哪个任务。
*            FreeRTOS 的可以给用户提供多个任务单独享有独立的堆栈空间，系统可以决定任务的状
*        态，决定任务是否可以运行， 同时还能运用内核的 IPC 通信资源，实现了任务之间的通信，
*        帮助用户管理业务程序流程。这样用户可以将更多的精力投入到业务功能的实现中。
*            FreeRTOS 中的任务是抢占式调度机制，高优先级的任务可打断低优先级任务，低优先级
*        任务必须在高优先级任务阻塞或挂起或删除后才能得到调度。 同时 FreeRTOS 也支持时间片
*        轮转调度方式，只不过时间片的调度是不允许抢占任务的 CPU 使用权。(注意：在系统中除了
*        中断处理函数、调度器上锁部分的代码和禁止中断的代码是不可抢占的之外，系统的其他部分
*        都是可以抢占的。)，任务优先级值越大，优先级也高，一般强制限定最大可用优先级数目为32
*        也就是一般情况下优先级值最高为31(当然这不是不能超过是这个数，这是可以修改的，但强烈
*        推荐不超过32)，在系统中，当有比当前任务优先级更高的任务就绪时，当前任务将立刻被换出，
*        高优先级任务抢占处理器运行。
*            FreeRTOS 内核中也允许创建相同优先级的任务。相同优先级的任务采用时间片轮转方式进
*        行调度（也就是通常说的分时调度器），时间片轮转调度仅在当前系统中无更高优先级就绪任务
*        存在的情况下才有效。
*
*		2、任务的四种状态：
*           运行态、就绪态、挂起态、阻塞态
*
*           就绪态（ Ready）：该任务在就绪列表中， 就绪的任务已经具备执行的能力，只等待调度器
*       进行调度，新创建的任务会初始化为就绪态。
*           运行（Running）：该状态表明任务正在执行， 此时它占用处理器， FreeRTOS 调度器选择
*       运行的永远是处于最高优先级的就绪态任务，当任务被运行的一刻，它的任务状态就变成了运行态。
*           阻塞（Blocked）： 如果任务当前正在等待某个时序或外部中断，我们就说这个任务处于阻塞
*       状态，该任务不在就绪列表中。包含任务被挂起、 任务被延时、任务正在等待信号量、读写队列或
*       者等待读写事件等。
*           挂起态(Suspended)： 处于挂起态的任务对调度器而言是不可见的，让一个任务进入挂起状态
*       的唯一办法就是调用 vTaskSuspend()函数；而把一个挂起状态的任务恢复的唯一途径就是调用
*       vTaskResume()或vTaskResumeFromISR()函数，我们可以这么理解挂起态与阻塞态的区别，当任务
*       有较长的时间不允许运行的时候，我们可以挂起任务，这样子调度器就不会管这个任务的任何信息，
*       直到我们调用恢复任务的API函数；而任务处于阻塞态的时候，系统还需要判断阻塞态的任务是否超
*       时，是否可以解除阻塞。
*
*		3、需要了解的函数：
*		vTaskSuspend(任务句柄)
* 					通过任务句柄挂起指定任务，被挂起的任务绝不会得到 CPU 的使用权，不管该任务具有什么优
*       先级。将任务由 运行态或就绪态或阻塞态——> 转变为挂起态
*       (注意：想要使用任务恢复函数 vTaskSuspend()则必须将宏定义INCLUDE_vTaskSuspend 配置为 1)
*
*   vTaskSuspendAll(void)
*           将所有的任务都挂起，这个函数就是比较有意思的，其实源码很简单，也很有意思，不管三七二
*       十一将调度器锁定，并且这个函数是可以进行嵌套的，说白了挂起所有任务就是挂起任务调度器。
*       vTaskSuspendAll()导致的挂起只能通过xTaskResumeAll()解除挂起。
*       (注意：vTaskSuspendAll()可以多次被调用，但要恢复时调用了多少次的vTaskSuspendAll()就要
*       调用多少次xTaskResumeAll()进行恢复)
*        
*   vTaskResume(任务句柄)
*           通过任务句柄恢复被挂起的任务，恢复的任务会保留挂起前的状态信息，在恢复的时候根据挂
*       起时的状态继续运行。
*       (注意：想要使用任务恢复函数 vTaskResume()则必须将宏定义INCLUDE_vTaskSuspend 配置为 1)
*
*   xTaskResumeFromISR(任务句柄)  
*							返回值：pdTRUE中断结束后需要切换任务，pdFALSE中断结束后不需要切换任务
*            xTaskResumeFromISR()的效果和vTaskResume()一样，都是通过任务句柄指定恢复被挂起的任务。
*       但区别在于xTaskResumeFromISR()是专门放在中断中使用的。
*       (注意：要想使用该函数必须在FreeRTOSConfig.h中把INCLUDE_vTaskSuspend和INCLUDE_vTaskResumeFromISR
*       都定义为1有效)
*       注意！！！：
*       1)、当函数的返回值为 pdTRUE 时： 恢复运行的任务的优先级等于或高于正在运行的任务，表明在中
*           断服务函数退出后必须进行一次上下文切换，使用portYIELD_FROM_ISR()进行上下文切换。当函
*           数的返回值为 pdFALSE 时： 恢复运行的任务的优先级低于当前正在运行的任务，表明在中断服
*           务函数退出后不需要进行上下文切换。
*       2)、xTaskResumeFromISR() 通常被认为是一个危险的函数，因为它的调用并非是固定的，中断可能
*           随时来来临。所以， xTaskResumeFromISR()不能用于任务和中断间的同步，如果中断恰巧在任
*           务被挂起之前到达，这就会导致一次中断丢失（任务还没有挂起，调用 xTaskResumeFromISR()
*           函数是没有意义的，只能等下一次中断）。这种情况下，可以使用信号量或者任务通知来同步就
*           可以避免这种情况。
*   
*   xTaskResumeAll()
*            恢复被vTaskSuspendAll()挂起的所有任务(恢复任务调度器)，xTaskResumeAll()只能针对vTaskSuspendAll()
*       因为vTaskSuspendAll和xTaskResumeAll它们的本质不是挂起和恢复任务，而是挂起和恢复任务调度器
*
*   vTaskDelete(任务句柄或NULL或什么都不填)
*            vTaskDelete()用于删除一个任务。当一个任务删除另外一个任务时， 形参为要删除任
*       务创建时返回的任务句柄，如果是删除自身， 则形参为 NULL或什么都不填。
*       (注意：要想使用该函数必须在FreeRTOSConfig.h 中把 INCLUDE_vTaskDelete 定义为 1)
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


//LED任务句柄
static TaskHandle_t LED_Task_Handle=NULL;	
//LED任务的任务函数
static void LED_Task(void* parameter);

//KEY任务句柄
static TaskHandle_t KEY_Task_Handle=NULL;	
//KEY任务的任务函数
static void Key_Task(void* parameter);

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
	KEY_Init();
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
								
	//创建Key_Task任务
	xReturn = xTaskCreate(
								(TaskFunction_t	)Key_Task,		     //任务函数
								(const char* 	  )"Key_Task",		   //任务名称
								(uint32_t 		  )128,					     //任务堆栈大小
								(void* 		  	  )NULL,				     //传递给任务函数的参数
								(UBaseType_t 	  )2, 				       //任务优先级
								(TaskHandle_t*  )&KEY_Task_Handle);//任务句柄 
																			
	if(xReturn == pdPASS) printf("Key_Task任务创建成功!\r\n");
	
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
				printf("挂起LED任务！\r\n");
				vTaskSuspend(LED_Task_Handle);//挂起LED任务
				printf("挂起LED任务成功！\r\n");
				break;
			case 2: 
				printf("恢复LED任务！\r\n");
				vTaskResume(LED_Task_Handle);//恢复LED任务！
				printf("恢复LED任务成功！\r\n");
				break;
			default: break;
		}
		
		vTaskDelay(20);
	}
}

