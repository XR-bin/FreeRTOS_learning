#include "freeRTOS_demo.h"
/* FreeRTOS相关文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"



/**
******************************************************************************
* @file      ：.\User\src\freeRTOS_demo.c
*              .\User\inc\freeRTOS_demo.h
* @author    ：XRbin
* @version   ：V1.0
* @date      ：2024-02-16
* @brief     ：freeRTOS实验代码
******************************************************************************
* @attention
*   我的GitHub   ：https://github.com/XR-bin
*   我的gitee    ：https://gitee.com/xrbin
*   我的leetcode ：https://leetcode.cn/u/xrbin/
******************************************************************************
*/



/**************************************************************************/
/* FreeRTOS二值信号量配置 */

QueueHandle_t semphore_handle;   /* semphore_handle二值信号量句柄 */



/* freeRTOS任务配置 */

/***
* 任务创建任务配置
* 包括: 任务优先级 堆栈大小 任务句柄 创建任务
*/
#define START_PRIO      1                     /* 任务优先级 */
#define START_STK_SIZE  64                    /* 任务堆栈大小，字为单位，1字等于4字节 */
static TaskHandle_t StartTask_Handler=NULL;   /* 创建任务的任务句柄 */
static void StartTaskCreate(void* parameter); /* 创建任务的任务函数名 */

/***
* TASK1任务配置
* 包括: 任务优先级 堆栈大小 任务句柄 创建任务
*/
#define TASK1_PRIO      1              /* 任务优先级 */
#define TASK1_STK_SIZE  64             /* 任务堆栈大小，字为单位，1字等于4字节 */
static TaskHandle_t Task1_Handle=NULL; /* 任务1的任务句柄 */
static void Task1(void* parameter);    /* 任务1的任务函数名 */

/***
* TASK2任务配置
* 包括: 任务优先级 堆栈大小 任务句柄 创建任务
*/
#define TASK2_PRIO      2              /* 任务优先级 */
#define TASK2_STK_SIZE  64             /* 任务堆栈大小，字为单位，1字等于4字节 */
static TaskHandle_t Task2_Handle=NULL; /* 任务2的任务句柄 */
static void Task2(void* parameter);    /* 任务2的任务函数名 */

/***
* TASK3任务配置
* 包括: 任务优先级 堆栈大小 任务句柄 创建任务
*/
#define TASK3_PRIO      3              /* 任务优先级 */
#define TASK3_STK_SIZE  64             /* 任务堆栈大小，字为单位，1字等于4字节 */
static TaskHandle_t Task3_Handle=NULL; /* 任务3的任务句柄 */
static void Task3(void* parameter);    /* 任务3的任务函数名 */

/**************************************************************************/





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
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值 pdPASS：成功 */

    /* 创建二值信号量 */
    semphore_handle = xSemaphoreCreateBinary();
    if(semphore_handle != NULL) printf("二值信号量创建成功！！！\r\n");
    else                        printf("二值信号量创建失败！！！\r\n");

    /* 动态创建创建任务的任务 */
    xReturn = xTaskCreate(
                         (TaskFunction_t )StartTaskCreate,     /* 任务函数 */
                         (const char*    )"StartTaskCreate",   /* 任务名称 */
                         (uint32_t       )START_STK_SIZE,      /* 任务堆栈大小 */
                         (void*          )NULL,                /* 传递给任务函数的参数 */
                         (UBaseType_t    )START_PRIO,          /* 任务优先级 */
                         (TaskHandle_t*  )&StartTask_Handler); /* 任务句柄 */

    if(xReturn == pdPASS) printf("StartTaskCreate任务创建成功!\r\n");
    else                  printf("StartTaskCreate任务创建失败!\r\n");

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
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值 pdPASS：成功 */

    taskENTER_CRITICAL();        /* 进入临界区，创建任务过程我们必须保证在临界区 */

    /* 动态创建Task1任务 */
    xReturn = xTaskCreate(
                         (TaskFunction_t )Task1,             /* 任务函数 */
                         (const char*    )"Task1",           /* 任务名称 */
                         (uint32_t       )TASK1_STK_SIZE,    /* 任务堆栈大小 */
                         (void*          )NULL,              /* 传递给任务函数的参数 */
                         (UBaseType_t    )TASK1_PRIO,        /* 任务优先级 */
                         (TaskHandle_t*  )&Task1_Handle);    /* 任务句柄 */

    if(xReturn == pdPASS) printf("Task1任务创建成功!\r\n");
    else                  printf("Task1任务创建失败!\r\n");

    /* 动态创建Task2任务 */
    xReturn = xTaskCreate(
                         (TaskFunction_t )Task2,             /* 任务函数 */
                         (const char*    )"Task2",           /* 任务名称 */
                         (uint32_t       )TASK2_STK_SIZE,    /* 任务堆栈大小 */
                         (void*          )NULL,              /* 传递给任务函数的参数 */
                         (UBaseType_t    )TASK2_PRIO,        /* 任务优先级 */
                         (TaskHandle_t*  )&Task2_Handle);    /* 任务句柄 */

    if(xReturn == pdPASS) printf("Task2任务创建成功!\r\n");
    else                  printf("Task2任务创建失败!\r\n");

    /* 动态创建Task3任务 */
    xReturn = xTaskCreate(
                         (TaskFunction_t )Task3,             /* 任务函数 */
                         (const char*    )"Task3",           /* 任务名称 */
                         (uint32_t       )TASK3_STK_SIZE,    /* 任务堆栈大小 */
                         (void*          )NULL,              /* 传递给任务函数的参数 */
                         (UBaseType_t    )TASK3_PRIO,        /* 任务优先级 */
                         (TaskHandle_t*  )&Task3_Handle);    /* 任务句柄 */

    if(xReturn == pdPASS) printf("Task3任务创建成功!\r\n");
    else                  printf("Task3任务创建失败!\r\n");

    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
    taskEXIT_CRITICAL();            /* 退出临界区 */
}



/**********************************************************
* @funcName ：Task1
* @brief    ：任务1
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：低优先级任务
************************************************************/
static void Task1(void* parameter)
{
    while(1)
    {
        printf("low_task获取信号量\r\n");
        xSemaphoreTake(semphore_handle,portMAX_DELAY);
        printf("low_task正在运行！！！\r\n");
        delay_ms(3000);
        printf("low_task释放信号量\r\n");
        xSemaphoreGive(semphore_handle); 
        vTaskDelay(1000);
    }
}



/**********************************************************
* @funcName ：Task2
* @brief    ：任务2
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：中等优先级任务
************************************************************/
static void Task2(void* parameter)
{
    while(1)
    {
        printf("middle_task正在运行！！！\r\n");
        vTaskDelay(1000);
    }
}



/**********************************************************
* @funcName ：Task3
* @brief    ：任务3
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：高优先级任务
************************************************************/
static void Task3(void* parameter)
{
    while(1)
    {
        printf("high_task获取信号量\r\n");
        xSemaphoreTake(semphore_handle,portMAX_DELAY);
        printf("high_task正在运行！！！\r\n");
        delay_ms(1000);
        printf("high_task释放信号量\r\n");
        xSemaphoreGive(semphore_handle); 
        vTaskDelay(1000);
    }
}


