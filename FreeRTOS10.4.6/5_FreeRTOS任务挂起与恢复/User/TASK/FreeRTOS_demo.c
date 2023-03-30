#include "FreeRTOS_demo.h"
#include "key.h"

/*************************************************************
*                       AIP函数
*  vTaskSuspend()                 挂起任务
*  vTaskResume()                  恢复被挂起的任务
*  xTaskResumeFromISR()           在中断中恢复被挂起的任务
*
*  挂起：类似于暂停的操作，是可恢复的。挂起不会进去阻塞列表，
*        而是进入挂起列表，挂起列表不会像阻塞列表一样别轮询，
*        只有当使用恢复任务函数时程序才会从挂起列表中将任务
*        拿出
*
*  注意：
*      1、在任务中恢复挂起任务用vTaskResume(),中断中恢复挂起
*         任务用xTaskResumeFromISR()
*      2、函数名带有FromISR的函数，表示是中断中专用的函数
*      3、中断服务函数在使用带有FromISR的函数时，该中断的优先
*         级不得高于FreeRTOS所管理的优先级(即必须是FreeRTOS管
*         理的中断才能使用FreeRTOS的函数)
***************************************************************/


/*************************************************************************
*                  vTaskSuspend()挂起任务API介绍
*
*      此函数用于挂起任务，若使用此函数，需要在FreeRTOSConfig.h文件中将宏
*  INCLUDE_vTaskSuspend 配置为 1。 无论优先级如何，被挂起的任务都将不再被
*  执行，直到任务被恢复。此函数并不支持嵌套，不论使用此函数重复挂起任务多少
*  次，只需调用一次恢复任务的函数，那么任务就不再被挂起。
*
*  函数原型：
*         void vTaskSuspend(TaskHandle_t xTaskToSuspend)
*
*  函数参数：
*         xTaskToSuspend       待挂起任务的任务句柄
*
*  函数返回值： 无
*
*  注意：当传入的参数是NULL，则表示挂起当前运行的任务自身。
***************************************************************************/

/*************************************************************************
*                  vTaskResume()恢复任务API介绍
*
*      此函数用于在任务中恢复被挂起的任务，若使用此函数，需要在FreeRTOSConfig.h
*  文件中将宏 INCLUDE_vTaskSuspend 配置为 1。不论一个任务被函数 vTaskSuspend()
*  挂起多少次，只需要使用函数 vTakResume()恢复一次，就可以继续运行。
*
*  函数原型：
*         void vTaskResume(TaskHandle_t xTaskToResume)
*
*  函数参数：
*         xTaskToResume        待恢复任务的任务句柄
*
*  函数返回值： 无
***************************************************************************/

/*************************************************************************
*               xTaskResumeFromISR()恢复任务(中断级)API介绍
*
*      此函数用于在中断中恢复被挂起的任务，若使用此函数，需要在FreeRTOSConfig.h
*  文件中将宏 INCLUDE_xTaskResumeFromISR 配置为 1。不论一个任务被函数vTaskSuspend()
*  挂起多少次，只需要使用函数vTakResumeFromISR()恢复一次，就可以继续运行。
*
*  函数原型：
*         BaseType_t xTaskResumeFromISR(TaskHandle_t xTaskToResume)
*
*  函数参数：
*         xTaskToResume        待恢复任务的任务句柄
*
*  函数返回值： 
*         pdTRUE        任务恢复后需要进行任务切换
*         pdFALSE       任务恢复后不需要进行任务切换
*
*  注意：在中断中使用xTaskResumeFromISR()必须判断是否要进行切换任务，如果用
*        就调用portEND_SWITCHING_ISR(x)(x填xTaskResumeFromISR()的返回值)进
*        行任务切换。
***************************************************************************/

/**
* 任务创建任务配置
* 包括: 任务优先级 堆栈大小 任务句柄 创建任务
*/
#define START_PRIO      1                     /* 任务优先级 */
#define START_STK_SIZE  64                    /* 任务堆栈大小 */
static TaskHandle_t StartTask_Handler=NULL;   /* 创建任务的任务句柄 */
static void StartTaskCreate(void* parameter); /* 创建任务的任务函数名 */

/**
* TASK1任务配置
* 包括: 任务优先级 堆栈大小 任务句柄 创建任务
*/
#define TASK1_PRIO      1              /* 任务优先级 */
#define TASK1_STK_SIZE  64             /* 任务堆栈大小 */
static TaskHandle_t Task1_Handle=NULL; /* 任务1的任务句柄 */
static void Task1(void* parameter);    /* 任务1的任务函数名 */
/**
* TASK2任务配置
* 包括: 任务优先级 堆栈大小 任务句柄 创建任务
*/
#define TASK2_PRIO      2              /* 任务优先级 */
#define TASK2_STK_SIZE  64             /* 任务堆栈大小 */
static TaskHandle_t Task2_Handle=NULL; /* 任务2的任务句柄 */
static void Task2(void* parameter);    /* 任务2的任务函数名 */
/**
* TASK3任务配置
* 包括: 任务优先级 堆栈大小 任务句柄 创建任务
*/
#define TASK3_PRIO      3              /* 任务优先级 */
#define TASK3_STK_SIZE  64             /* 任务堆栈大小 */
static TaskHandle_t Task3_Handle=NULL; /* 任务3的任务句柄 */
static void Task3(void* parameter);    /* 任务3的任务函数名 */


/**********************************************************
*@funcName ：App_demo
*@brief    ：学习的事例demo
*@param    ：void(无)
*@retval   ：void(无)
*@fn       ：
************************************************************/
void App_demo(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值 pdPASS：成功 */
    
    /* 动态创建创建任务的任务 */
    xReturn = xTaskCreate(
                        (TaskFunction_t	)StartTaskCreate,	  /* 任务函数 */
                        (const char* 	)"StartTaskCreate",	  /* 任务名称 */
                        (uint32_t 		)START_STK_SIZE,	  /* 任务堆栈大小 */
                        (void* 		  	)NULL,				  /* 传递给任务函数的参数 */
                        (UBaseType_t 	)START_PRIO, 		  /* 任务优先级 */
                        (TaskHandle_t*  )&StartTask_Handler); /* 任务句柄 */
                        
    if(xReturn == pdPASS) printf("StartTaskCreate任务创建成功!\r\n");
    else                  printf("StartTaskCreate任务创建失败!\r\n");
    
    vTaskStartScheduler();  /* 启动任务调度器 */
}

/**********************************************************
*@funcName ：StartTaskCreate
*@brief    ：用于创建任务的任务
*@param    ：void* parameter(未用到)
*@retval   ：void(无)
*@fn       ：
*            这个任务创建函数是专门用来创建任务函数的，我
*        们会把它当任务创建，当它把其他任务创建完成后，我
*        们会我们会把该任务销毁。
************************************************************/
static void StartTaskCreate(void* parameter)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值 pdPASS：成功 */
    
    taskENTER_CRITICAL(); /* 进入临界区，创建任务过程我们必须保证在临界区 */
    
    /* 动态创建Task1任务 */
    xReturn = xTaskCreate(
                        (TaskFunction_t	)Task1,		        /* 任务函数 */
                        (const char* 	)"Task1",		    /* 任务名称 */
                        (uint32_t 		)TASK1_STK_SIZE,	/* 任务堆栈大小 */
                        (void* 		  	)NULL,				/* 传递给任务函数的参数 */
                        (UBaseType_t 	)TASK1_PRIO, 		/* 任务优先级 */
                        (TaskHandle_t*  )&Task1_Handle);    /* 任务句柄 */

	if(xReturn == pdPASS) printf("Task1任务创建成功!\r\n");
	else                  printf("Task1任务创建失败!\r\n");
    
    /* 动态创建Task2任务 */
    xReturn = xTaskCreate(
                        (TaskFunction_t	)Task2,		        /* 任务函数 */
                        (const char* 	)"Task2",		    /* 任务名称 */
                        (uint32_t 		)TASK2_STK_SIZE,	/* 任务堆栈大小 */
                        (void* 		  	)NULL,				/* 传递给任务函数的参数 */
                        (UBaseType_t 	)TASK2_PRIO, 		/* 任务优先级 */
                        (TaskHandle_t*  )&Task2_Handle);    /* 任务句柄 */

	if(xReturn == pdPASS) printf("Task2任务创建成功!\r\n");
	else                  printf("Task2任务创建失败!\r\n");
    
    /* 动态创建Task3任务 */
    xReturn = xTaskCreate(
                        (TaskFunction_t	)Task3,		        /* 任务函数 */
                        (const char* 	)"Task3",		    /* 任务名称 */
                        (uint32_t 		)TASK3_STK_SIZE,	/* 任务堆栈大小 */
                        (void* 		  	)NULL,				/* 传递给任务函数的参数 */
                        (UBaseType_t 	)TASK3_PRIO, 		/* 任务优先级 */
                        (TaskHandle_t*  )&Task3_Handle);    /* 任务句柄 */

	if(xReturn == pdPASS) printf("Task3任务创建成功!\r\n");
	else                  printf("Task3任务创建失败!\r\n");
    
    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
    taskEXIT_CRITICAL();            /* 退出临界区 */
}

/*********************************************************************
*@funcName ：Task1
*@brief    ：任务1
*@param    ：void* parameter(未用到)
*@retval   ：void(无)
*@fn       ：用动态创建任务的方式创建的任务1
**********************************************************************/
static void Task1(void* parameter)
{
    while (1)
    {
        printf("动态创建任务1\r\n");
        vTaskDelay(900);   /* 延时500个tick */
    }
}

/*********************************************************************
*@funcName ：Task2
*@brief    ：任务2
*@param    ：void* parameter(未用到)
*@retval   ：void(无)
*@fn       ：用动态创建任务的方式创建的任务2
**********************************************************************/
static void Task2(void* parameter)
{
    while (1)
    {
        printf("动态创建任务2\r\n");
        vTaskDelay(900);   /* 延时500个tick */
    }
}

/*********************************************************************
*@funcName ：Task3
*@brief    ：任务3
*@param    ：void* parameter(未用到)
*@retval   ：void(无)
*@fn       ：用动态创建任务的方式创建的任务3
**********************************************************************/
static void Task3(void* parameter)
{
    uint8_t key = 0;
    
    while (1)
    {
        key = KEY_Scan();
        switch(key)
        {
            case 1:  /* KEY0按下 */
                vTaskSuspend(Task1_Handle);
                printf("挂起任务1\r\n");
                break;
            case 2:  /* KEY1按下 */
                vTaskResume(Task1_Handle);
                printf("恢复任务1\r\n");
                break;
            default: break;
        }
        vTaskDelay(10);   /* 延时500个tick */
    }
}

