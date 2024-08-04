#include "FreeRTOS.h"
#include "task.h"

/******************************************************
*
*	【注意】
*				在FreeRTOS中，优先级数越小，逻辑优先级越低
*   而RT-Thread和uC/OS正好相反，优先级数越小，逻辑
*   优先级越高
*
*******************************************************/

ListItem_t xStateListItem;

//定义两个全局变量
portCHAR flag1;
portCHAR flag2;

//定义任务栈
//创建两个128字大小的任务栈
//1个字等于4个字节
//FreeRTOS推荐的最小任务栈为512个字节
/*任务1的栈*/
#define TASK1_STACK_SIZE  128
StackType_t Task1Stack[TASK1_STACK_SIZE];
TCB_t Task1TCB;  //任务1控制块
TaskHandle_t Task1_Handle; //任务1句柄
void Task1_Entry( void *p_arg );    //任务1函数申明

/*任务2的栈*/
#define TASK2_STACK_SIZE  128
StackType_t Task2Stack[TASK2_STACK_SIZE];
TCB_t Task2TCB;  //任务2控制块
TaskHandle_t Task2_Handle;    //任务2句柄
void Task2_Entry( void *p_arg );    //任务2函数申明



int main(void)
{
	/* 硬件初始化 */
	/* 将硬件相关的初始化放在这里，如果是软件仿真则没有相关初始化代码 */

	/* 创建任务 */
	Task1_Handle = xTaskCreateStatic( (TaskFunction_t)Task1_Entry,   /* 任务入口 */
																		(char *)"Task1",               /* 任务名称，字符串形式 */
																		(uint32_t)TASK1_STACK_SIZE ,   /* 任务栈大小，单位为字 */
																		(void *) NULL,                 /* 任务形参 */
																		(UBaseType_t) 1,               /* 任务优先级，数值越大，优先级越高 */
																		(StackType_t *)Task1Stack,     /* 任务栈起始地址 */
																		(TCB_t *)&Task1TCB );          /* 任务控制块 */
																		
	Task2_Handle = xTaskCreateStatic( (TaskFunction_t)Task2_Entry,   /* 任务入口 */
																		(char *)"Task2",               /* 任务名称，字符串形式 */
																		(uint32_t)TASK2_STACK_SIZE ,   /* 任务栈大小，单位为字 */
																		(void *) NULL,                 /* 任务形参 */
																		(UBaseType_t) 2,               /* 任务优先级，数值越大，优先级越高 */
																		(StackType_t *)Task2Stack,     /* 任务栈起始地址 */
																		(TCB_t *)&Task2TCB );          /* 任务控制块 */																
	
	/* 在启动调度器前，关闭中断 */                                  
	portDISABLE_INTERRUPTS();
	/* 启动调度器，开始多任务调度，启动成功则不返回 */
  vTaskStartScheduler();  
																		
	while(1);
}

/***************************任务内容***************************/
void delay (uint32_t count)
{
	for(; count!=0; count--);
}

/*任务1*/
void Task1_Entry( void *p_arg )
{
	while(1)
	{
		flag1 = 1;
		vTaskDelay(10);		
		flag1 = 0;
		vTaskDelay(10);
	}
}

/* 任务2 */
void Task2_Entry( void *p_arg )
{
	while(1)
	{
		flag2 = 1;
		vTaskDelay(10);		
		flag2 = 0;
		vTaskDelay(10);
	}
}


/* 获取空闲任务的内存 */
StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];   //空闲任务栈
TCB_t IdleTaskTCB;                                     //空闲任务控制块

void vApplicationGetIdleTaskMemory( TCB_t **ppxIdleTaskTCBBuffer, 
                                    StackType_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize )
{
		*ppxIdleTaskTCBBuffer=&IdleTaskTCB;             //空闲任务控制块
		*ppxIdleTaskStackBuffer=IdleTaskStack;          //空闲任务栈
		*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE; //空闲任务栈大小
}
