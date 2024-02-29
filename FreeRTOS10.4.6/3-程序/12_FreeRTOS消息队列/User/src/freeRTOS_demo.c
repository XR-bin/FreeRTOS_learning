#include "freeRTOS_demo.h"
#include "key.h"
/* FreeRTOS相关文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"



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
/* FreeRTOS消息队列配置 */

QueueHandle_t key_queue;        /* 小数据句柄 */
QueueHandle_t big_date_queue;   /* 大数据句柄 */




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

    /* key_queue队列的创建 */
    key_queue = xQueueCreate(2, sizeof(uint8_t));              /* 队列长度为2，成员大小为sizeof(uint8_t) */
    if(key_queue != NULL)  printf("key_queue队列创建成功！！\r\n");
    else                   printf("key_queue队列创建失败！！\r\n");

    /* big_date_queue队列的创建 */
    big_date_queue = xQueueCreate( 1, sizeof(char *) );        /* 队列长度为1，成员大小为sizeof(char*) */
    if(big_date_queue != NULL) printf("big_date_queue队列创建成功！！\r\n");
    else                       printf("big_date_queue队列创建失败！！\r\n");

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



char buff[100] = {"我是一个大数组，大大的数组 124214 uhsidhaksjhdklsadhsaklj"};
/**********************************************************
* @funcName ：Task1
* @brief    ：任务1
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：通过按键KEY发送消息到指定消息队列中
************************************************************/
static void Task1(void* parameter)
{
    uint8_t key = 0;
    char * buf;
    BaseType_t err = 0;
    buf = &buff[0];

    while(1)
    {
        key = KEY_Scan();
        switch(key)
        {
            /* KEY0按下 */
            case 1:
                    err = xQueueSend(key_queue, &key, portMAX_DELAY);
                    if(err != pdTRUE) printf("key_queue队列发送失败\r\n");
                    else              printf("key_queue队列发送成功\r\n");
                    break;

            /* KEY1按下 */
            case 2:
                    err = xQueueSend(big_date_queue, &buf, portMAX_DELAY);
                    if(err != pdTRUE)printf("key_queue队列发送失败\r\n");
                    else             printf("key_queue队列发送成功\r\n");
                    break;

            default: break;
        }

        vTaskDelay(10);   /* 延时10个tick */
    }
}



/**********************************************************
* @funcName ：Task2
* @brief    ：任务2
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：获取key_queue消息队列里的消息
************************************************************/
static void Task2(void* parameter)
{
    uint8_t key = 0;
    BaseType_t err = 0;

    while(1)
    {
        err = xQueueReceive( key_queue,&key,portMAX_DELAY);
        if(err != pdTRUE) printf("key_queue队列读取失败\r\n");
        else              printf("key_queue读取队列成功，数据：%d\r\n",key);

        vTaskDelay(100);   /* 延时100个tick */
    }
}



/**********************************************************
* @funcName ：Task3
* @brief    ：任务3
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：获取big_date_queue消息队列里的消息
************************************************************/
static void Task3(void* parameter)
{
    char * buf;
    BaseType_t err = 0;

    while(1)
    {
        err = xQueueReceive(big_date_queue, &buf,portMAX_DELAY);
        if(err != pdTRUE) printf("big_date_queue队列读取失败\r\n");
        else              printf("数据：%s\r\n", buf);

        vTaskDelay(100);   /* 延时100个tick */
    }
}

