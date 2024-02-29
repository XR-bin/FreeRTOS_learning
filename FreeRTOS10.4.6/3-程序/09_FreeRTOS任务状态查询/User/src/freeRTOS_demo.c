#include "freeRTOS_demo.h"
#include "led.h"
#include "malloc.h"
/* FreeRTOS相关文件 */
#include "FreeRTOS.h"
#include "task.h"



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



char task_buff[500];   /* 清单信息缓存区 */
/**********************************************************
* @funcName ：Task1
* @brief    ：任务1
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：使用查询FreeRTOS任务状态函数
************************************************************/
static void Task1(void* parameter)
{
    UBaseType_t priority_num = 0;          /* 任务优先级值 */
    UBaseType_t task_num = 0;              /* 任务数量 */
    UBaseType_t task_num2 = 0;             /* 任务数量 */
    TaskStatus_t * status_array = 0;       /* 所有任务状态信息 */
    TaskStatus_t * status_array2 = 0;      /* 指定的任务所有状态信息 */
    TaskHandle_t task_handle = 0;          /* 某个任务的任务句柄 */
    UBaseType_t task_stack_min = 0;        /* 剩余堆栈大小 */
    int state = 0;                  /* 指定任务的状态信息 */

    uint8_t i = 0;

    /* 修改指定任务的优先级 */
    vTaskPrioritySet(Task2_Handle, 4);

    /* 获取指定任务优先级值，NULL表示获取自身的 */
    priority_num = uxTaskPriorityGet( NULL );
    printf("task1任务优先级为%ld\r\n", priority_num);

    /* 获取当前系统中任务数量 */
    task_num = uxTaskGetNumberOfTasks();
    printf("任务数量：%ld\r\n",task_num);

    /* 获取所有任务的状态信息 */
    status_array = mymalloc(0,(sizeof(TaskStatus_t) * task_num));   /* 在SRAM的内存管理中申请空间 */
    task_num2 = uxTaskGetSystemState( status_array,task_num,NULL);
    printf("任务名\t\t任务优先级\t任务编号\r\n");
    for(i = 0; i < task_num2; i++)
    {
        printf("%s\t\t%ld\t%ld\r\n",
                status_array[i].pcTaskName,
                status_array[i].uxCurrentPriority,
                status_array[i].xTaskNumber);
    }

    /* 获取指定任务的所有信息v*/
    status_array2 = mymalloc(SRAMIN,sizeof(TaskStatus_t));
    vTaskGetInfo(Task2_Handle, status_array2, pdTRUE,eInvalid);
    printf("任务名：%s\r\n",status_array2->pcTaskName);
    printf("任务优先级：%ld\r\n",status_array2->uxCurrentPriority);
    printf("任务编号：%ld\r\n",status_array2->xTaskNumber);
    printf("任务状态：%d\r\n",status_array2->eCurrentState);

    /* 通过任务名获取任务句柄 */
    task_handle = xTaskGetHandle( "Task1" );
    printf("任务句柄：%#x\r\n",(int)task_handle);
    printf("task1的任务句柄：%#x\r\n",(int)Task1_Handle);

    /* 获取指定任务的状态 */
    eTaskGetState(Task2_Handle);
    state = eTaskGetState(Task2_Handle);
    printf("当前task2的任务状态为：%d\r\n",state);

    /* 以清单形式获取所有任务状态信息 */
    vTaskList( task_buff );
    printf("%s\r\n",task_buff);

    while(1)
    {
        /* 获取剩余堆栈大小 */
        task_stack_min = uxTaskGetStackHighWaterMark(Task2_Handle);
        printf("task2历史剩余最小堆栈为%ld\r\n",task_stack_min);
        vTaskDelay(800);   /* 延时800个tick */
    }
}



/**********************************************************
* @funcName ：Task2
* @brief    ：任务2
* @param    ：void* parameter(传入参数,未用到)
* @retval   ：void
* @details  ：
* @fn       ：就只做LED翻转，证明系统正常运行
************************************************************/
static void Task2(void* parameter)
{
    while(1)
    {
        GPIOB->ODR ^= (1<<5);     /* LED0翻转 */
        GPIOE->ODR ^= (1<<5);     /* LED1翻转 */
        vTaskDelay(800);          /* 延时800个tick */
    }
}




