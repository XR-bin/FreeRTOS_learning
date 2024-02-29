#include "freeRTOS_demo.h"
/* FreeRTOS相关文件 */
#include "FreeRTOS.h"
#include "task.h"



/**
******************************************************************************
* @file      ：.\User\src\freeRTOS_demo.c
*              .\User\inc\freeRTOS_demo.h
* @author    ：XRbin
* @version   ：V1.0
* @date      ：2024-01-02
* @brief     ：freeRTOS实验代码
******************************************************************************
* @attention
*   我的GitHub   ：https://github.com/XR-bin
*   我的gitee    ：https://gitee.com/xrbin
*   我的leetcode ：https://leetcode.cn/u/xrbin/
******************************************************************************
*/



/**************************************************************************/
/* freeRTOS任务配置 */

/***
* 空闲任务和定时器服务任务配置
* 包括: 任务堆栈 任务控制块
*/
static StackType_t  IdleTaskStack[configMINIMAL_STACK_SIZE];        /* 空闲任务任务堆栈 */
static StaticTask_t IdleTaskTCB;                                    /* 空闲任务控制块 */
static StackType_t  TimerTaskStack[configTIMER_TASK_STACK_DEPTH];   /* 定时器服务任务堆栈 */
static StaticTask_t TimerTaskTCB;                                   /* 定时器服务任务控制块 */

/***
* 任务创建任务配置
* 包括: 任务优先级 堆栈大小 任务堆栈 任务控制块 任务句柄 任务函数
*/
#define START_PRIO      1                     /* 任务优先级 */
#define START_STK_SIZE  64                    /* 任务堆栈大小，字为单位，1字等于4字节 */
StackType_t  StartTaskStack[START_STK_SIZE];  /* 任务堆栈 */
StaticTask_t StartTaskTCB;                    /* 任务控制块 */
static TaskHandle_t StartTask_Handler=NULL;   /* 创建任务的任务句柄 */
static void StartTaskCreate(void* parameter); /* 创建任务的任务函数名 */

/***
* TASK1任务配置
* 包括: 任务优先级 堆栈大小 任务堆栈 任务控制块 任务句柄 任务函数
*/
#define TASK1_PRIO      1                     /* 任务优先级 */
#define TASK1_STK_SIZE  64                    /* 任务堆栈大小，字为单位，1字等于4字节 */
StackType_t  Task1TaskStack[TASK1_STK_SIZE];  /* 任务堆栈 */
StaticTask_t Task1TaskTCB;                    /* 任务控制块 */
static TaskHandle_t Task1_Handle=NULL;        /* 任务1的任务句柄 */
static void Task1(void* parameter);           /* 任务1的任务函数名 */

/***
* TASK2任务配置
* 包括: 任务优先级 堆栈大小 任务堆栈 任务控制块 任务句柄 任务函数
*/
#define TASK2_PRIO      2                     /* 任务优先级 */
#define TASK2_STK_SIZE  64                    /* 任务堆栈大小，字为单位，1字等于4字节 */
StackType_t  Task2TaskStack[TASK2_STK_SIZE];  /* 任务堆栈 */
StaticTask_t Task2TaskTCB;                    /* 任务控制块 */
static TaskHandle_t Task2_Handle=NULL;        /* 任务2的任务句柄 */
static void Task2(void* parameter);           /* 任务2的任务函数名 */

/***
* TASK3任务配置
* 包括: 任务优先级 堆栈大小 任务堆栈 任务控制块 任务句柄 任务函数
*/
#define TASK3_PRIO      3                     /* 任务优先级 */
#define TASK3_STK_SIZE  64                    /* 任务堆栈大小，字为单位，1字等于4字节 */
StackType_t  Task3TaskStack[TASK3_STK_SIZE];  /* 任务堆栈 */
StaticTask_t Task3TaskTCB;                    /* 任务控制块 */
static TaskHandle_t Task3_Handle=NULL;        /* 任务3的任务句柄 */
static void Task3(void* parameter);           /* 任务3的任务函数名 */

/**************************************************************************/





/**********************************************************
* @funcName ：vApplicationGetIdleTaskMemory
* @brief    ：获取空闲任务地任务堆栈和任务控制块内存
* @param    ：ppxIdleTaskTCBBuffer  (任务控制块内存)
* @param    ：ppxIdleTaskStackBuffer(任务堆栈内存)
* @param    ：pulIdleTaskStackSize  (任务堆栈大小)
* @retval   ：void
* @details  ：
* @fn       ：
*               获取空闲任务地任务堆栈和任务控制块内存，因为本例程使用的
*           静态内存，因此空闲任务的任务堆栈和任务控制块的内存就应该有用
*           户来提供，FreeRTOS提供了接口函数vApplicationGetIdleTaskMemory()
*           实现此函数即可。
************************************************************/
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t  **ppxIdleTaskStackBuffer,
                                   uint32_t     *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer   = &IdleTaskTCB;
    *ppxIdleTaskStackBuffer = IdleTaskStack;
    *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}



/**********************************************************************
*@funcName ：vApplicationGetTimerTaskMemory
*@brief    ：获取定时器服务任务的任务堆栈和任务控制块内存
*@param    ：
*            ppxTimerTaskTCBBuffer    任务控制块内存
*            ppxTimerTaskStackBuffer  任务堆栈内存
*            pulTimerTaskStackSize    任务堆栈大小
*@retval   ：void
*@fn       ：
*               获取定时器服务任务的任务堆栈和任务控制块内存，因为本例程使用的
*           静态内存，因此定时器服务任务的任务堆栈和任务控制块的内存就应该有用
*           户来提供，FreeRTOS提供了接口函数vApplicationGetTimerTaskMemory()
*           实现此函数即可。
************************************************************************/
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t  **ppxTimerTaskStackBuffer,
                                    uint32_t     *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer  = &TimerTaskTCB;
    *ppxTimerTaskStackBuffer= TimerTaskStack;
    *pulTimerTaskStackSize  = configTIMER_TASK_STACK_DEPTH;
}



/**********************************************************
* @funcName ：freeRTOS_demo
* @brief    ：freeRTOS实验函数
* @param    ：void
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void freeRTOS_demo(void)
{
    /* 静态创建创建任务的任务 */
    StartTask_Handler = xTaskCreateStatic(
                                         (TaskFunction_t   )StartTaskCreate,   /* 任务函数 */
                                         (const char*      )"StartTaskCreate", /* 任务名称 */
                                         (uint32_t         )START_STK_SIZE,    /* 任务堆栈大小 */
                                         (void*            )NULL,              /* 传递给任务函数的参数 */
                                         (UBaseType_t      )START_PRIO,        /* 任务优先级 */
                                         (StackType_t*     )StartTaskStack,    /* 任务堆栈 */
                                         (StaticTask_t*    )&StartTaskTCB);    /* 任务控制块 */

    if(StartTask_Handler != NULL) printf("StartTaskCreate任务创建成功!\r\n");
    else                          printf("StartTaskCreate任务创建失败!\r\n");

    vTaskStartScheduler();  /* 启动任务调度器 */
}



/**********************************************************
* @funcName ：StartTaskCreate
* @brief    ：用于创建任务的任务
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：
*            这个任务创建函数是专门用来创建任务函数的，我
*        们会把它当任务创建，当它把其他任务创建完成后，我
*        们会我们会把该任务销毁。
************************************************************/
static void StartTaskCreate(void* parameter)
{
    taskENTER_CRITICAL(); /* 进入临界区，创建任务过程我们必须保证在临界区 */

    /* 静态创建Task1任务 */          
    Task1_Handle = xTaskCreateStatic(
                                    (TaskFunction_t   )Task1,             /* 任务函数 */
                                    (const char*      )"Task1",           /* 任务名称 */
                                    (uint32_t         )TASK1_STK_SIZE,    /* 任务堆栈大小 */
                                    (void*            )NULL,              /* 传递给任务函数的参数 */
                                    (UBaseType_t      )TASK1_PRIO,        /* 任务优先级 */
                                    (StackType_t*     )Task1TaskStack,    /* 任务堆栈 */
                                    (StaticTask_t*    )&Task1TaskTCB);    /* 任务控制块 */

    if(Task1_Handle != NULL) printf("Task1任务创建成功!\r\n");
    else                     printf("Task1任务创建失败!\r\n");

    /* 静态创建Task2任务 */
    Task2_Handle = xTaskCreateStatic(
                                    (TaskFunction_t   )Task2,             /* 任务函数 */
                                    (const char*      )"Task2",           /* 任务名称 */
                                    (uint32_t         )TASK2_STK_SIZE,    /* 任务堆栈大小 */
                                    (void*            )NULL,              /* 传递给任务函数的参数 */
                                    (UBaseType_t      )TASK2_PRIO,        /* 任务优先级 */
                                    (StackType_t*     )Task2TaskStack,    /* 任务堆栈 */
                                    (StaticTask_t*    )&Task2TaskTCB);    /* 任务控制块 */

    if(Task2_Handle != NULL) printf("Task2任务创建成功!\r\n");
    else                     printf("Task2任务创建失败!\r\n");

    /* 静态创建Task3任务 */
    Task3_Handle = xTaskCreateStatic(
                                    (TaskFunction_t   )Task3,             /* 任务函数 */
                                    (const char*      )"Task3",           /* 任务名称 */
                                    (uint32_t         )TASK3_STK_SIZE,    /* 任务堆栈大小 */
                                    (void*            )NULL,              /* 传递给任务函数的参数 */
                                    (UBaseType_t      )TASK3_PRIO,        /* 任务优先级 */
                                    (StackType_t*     )Task3TaskStack,    /* 任务堆栈 */
                                    (StaticTask_t*    )&Task3TaskTCB);    /* 任务控制块 */

    if(Task3_Handle != NULL) printf("Task3任务创建成功!\r\n");
    else                     printf("Task3任务创建失败!\r\n");

    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
    taskEXIT_CRITICAL();            /* 退出临界区 */
}



/**********************************************************
* @funcName ：Task1
* @brief    ：任务1
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：用静态创建任务的方式创建的任务1
************************************************************/
static void Task1(void* parameter)
{
    while(1)
    {
        printf("动态创建任务1\r\n");
        vTaskDelay(800);   /* 延时800个tick */
    }
}



/**********************************************************
* @funcName ：Task2
* @brief    ：任务2
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：用静态创建任务的方式创建的任务2
************************************************************/
static void Task2(void* parameter)
{
    while(1)
    {
        printf("动态创建任务2\r\n");
        vTaskDelay(800);   /* 延时800个tick */
    }
}



/**********************************************************
* @funcName ：Task3
* @brief    ：任务3
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：用静态创建任务的方式创建的任务3
************************************************************/
static void Task3(void* parameter)
{
    while(1)
    {
        printf("动态创建任务3\r\n");
        vTaskDelay(800);   /* 延时800个tick */
    }
}

