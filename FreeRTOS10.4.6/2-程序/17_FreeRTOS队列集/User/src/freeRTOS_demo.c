#include "freeRTOS_demo.h"
#include "key.h"
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
* @date      ：2024-02-18
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

QueueHandle_t semphore_handle;    /* semphore_handle二值信号量句柄 */



/* FreeRTOS队列配置 */

QueueHandle_t queue_handle;       /* queue_handle队列句柄 */



/* FreeRTOS队列集配置 */

QueueSetHandle_t queueset_handle; /* queueset_handle队列集句柄 */



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

    /* 创建消息队列 */
    queue_handle = xQueueCreate(1, sizeof(uint8_t));              /* 队列长度为1，成员大小为sizeof(uint8_t) */
    if(queue_handle != NULL)  printf("消息队列创建成功！！\r\n");
    else                      printf("消息队列创建失败！！\r\n");

    /* 创建队列集 */
    queueset_handle = xQueueCreateSet(2);                         /* 创建队列集，可以存放2个队列 */
    if(queueset_handle != NULL) printf("队列集创建成功！！\r\n");
    else                        printf("队列集创建失败！！\r\n");

    /* 将二值信号量和队列添加到队列集 */
    xQueueAddToSet(queue_handle, queueset_handle);
    xQueueAddToSet(semphore_handle, queueset_handle);

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

    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
    taskEXIT_CRITICAL();            /* 退出临界区 */
}



/**********************************************************
* @funcName ：Task1
* @brief    ：任务1
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：获取队列集中的消息
************************************************************/
static void Task1(void* parameter)
{
    QueueSetMemberHandle_t member_handle;
    uint8_t key;

    while(1)
    {
        /* 查询队列集信息 */
        member_handle = xQueueSelectFromSet(queueset_handle, portMAX_DELAY);

        /* 识别队列集中是二值信号量还是消息队列有数据 */
        if(member_handle == queue_handle)
        {
            /* 获取队列集中消息队列的数据 */
            xQueueReceive(member_handle, &key, portMAX_DELAY);
            printf("获取到的队列数据为：%d\r\n",key);
        }
        else if(member_handle == semphore_handle)
        {
            /* 获取队列集中二值信号量的数据 */
            xSemaphoreTake(member_handle, portMAX_DELAY);
            printf("获取信号量成功！！\r\n");
        }
    }
}



/**********************************************************
* @funcName ：Task2
* @brief    ：任务2
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：通过按键向消息队列发送数据或释放二值信号量
************************************************************/
static void Task2(void* parameter)
{
    uint8_t key = 0;
    BaseType_t err;

    while(1)
    {
        key = KEY_Scan();

        /* KEY0被按下 */
        if(key == 1)
        {
            if(semphore_handle != NULL)
            {
                /* 向队列集中的消息队列发送数据 */
                err = xQueueSend(queue_handle, &key, portMAX_DELAY);
                if(err == pdPASS) printf("往队列queue_handle写入数据成功！！\r\n");
                else              printf("往队列queue_handle写入数据失败！！\r\n");
            }
        }
        else if(key == 2)
        {
            err = xSemaphoreGive(semphore_handle);
            if(err == pdPASS) printf("释放信号量成功！！\r\n");
            else              printf("释放信号量失败！！\r\n");
        }

        vTaskDelay(10);   /* 延时10个tick */
    }
}




