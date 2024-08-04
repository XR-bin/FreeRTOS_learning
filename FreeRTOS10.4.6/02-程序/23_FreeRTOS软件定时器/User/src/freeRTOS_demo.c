#include "freeRTOS_demo.h"
#include "key.h"
/* FreeRTOS相关文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"



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
/* FreeRTOS软件定时器配置 */

/* 软件定时器1 */
void timer1_callback( TimerHandle_t pxTimer );
TimerHandle_t timer1_handle = 0;    /* 单次定时器计数 */

/* 软件定时器2 */
void timer2_callback( TimerHandle_t pxTimer );
TimerHandle_t timer2_handle = 0;    /* 周期定时器计数 */



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

    /* 软件定时器1 --- 单次定时器 */
    timer1_handle = xTimerCreate( "timer1", 
                                    500,
                                    pdFALSE,
                                    (void *)1,
                                    timer1_callback );

    /* 软件定时器2 --- 周期定时器 */
    timer2_handle = xTimerCreate( "timer2", 
                                    2000,
                                    pdTRUE,
                                    (void *)2,
                                    timer2_callback );

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

    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
    taskEXIT_CRITICAL();            /* 退出临界区 */
}



/**********************************************************
* @funcName ：Task1
* @brief    ：任务1
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：通过按键控制软件定时器的开启与关闭
************************************************************/
static void Task1(void* parameter)
{
    uint8_t key = 0;

    while(1)
    {
        key = KEY_Scan();

        switch(key)
        {
            /* KEY0按下 */
            case 1:
                    /* 开启软件定时器 */
                    xTimerStart(timer1_handle,portMAX_DELAY);
                    xTimerStart(timer2_handle,portMAX_DELAY);
                    break;

            /* KEY1按下 */
            case 2:
                    /* 关闭软件定时器 */
                    xTimerStop(timer1_handle,portMAX_DELAY);
                    xTimerStop(timer2_handle,portMAX_DELAY);
                    break;
        }

        vTaskDelay(10);   /* 延时10个tick */
    }
}



/**********************************************************
* @funcName ：timer1_callback
* @brief    ：软件定时器1的超时回调函数
* @param    ：TimerHandle_t pxTimer(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void timer1_callback(TimerHandle_t pxTimer)
{
    static uint32_t timer = 0;
    printf("timer1的运行次数：%d\r\n",++timer);
}



/**********************************************************
* @funcName ：timer2_callback
* @brief    ：软件定时器2的超时回调函数
* @param    ：TimerHandle_t pxTimer(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void timer2_callback(TimerHandle_t pxTimer)
{
    static uint32_t timer = 0;
    printf("timer2的运行次数：%d\r\n",++timer);
}





