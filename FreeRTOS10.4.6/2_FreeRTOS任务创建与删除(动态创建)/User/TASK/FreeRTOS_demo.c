#include "FreeRTOS_demo.h"

/*************************************************************
*                       AIP函数
*  xTaskCreate()                  动态方式创建任务
*  xTaskCreateStatic()            静态方式创建任务
*  xTaskCreateRestricted()        动态方式创建使用 MPU 限制的任务
*  xTaskCreateRestrictedStatic()  静态方式创建使用 MPU 限制的任务
*  vTaskDelete()                  删除任务
*
*  注：xTaskCreateRestricted()和xTaskCreateRestrictedStatic()比较
*      少用，只有项目有要求任务需要MPU保护才会使用，所有我就不对这两
*      个创建任务的API进行了解，等需要用时我再做了解。
***************************************************************/


/*************************************************************************
*                   xTaskCreate()动态创建任务API介绍
*
*      此函数用于使用动态的方式创建任务，任务的任务控制块以及任务的栈空间
*  所需的内存，均由 FreeRTOS 从 FreeRTOS 管理的堆中分配，若使用此函数，需
*  要在FreeRTOSConfig.h文件中将宏configSUPPORT_DYNAMIC_ALLOCATION配置为 1。
*  此函数创建的任务会立刻进入就绪态，由任务调度器调度运行。 
*
*  函数原型：
*         BaseType_t xTaskCreate(
*                TaskFunction_t                pxTaskCode,
*                const char *                  const pcName,
*                const configSTACK_DEPTH_TYPE  usStackDepth,
*                void * const                  pvParameters,
*                UBaseType_t                   uxPriority,
*                TaskHandle_t * const          pxCreatedTask);
*
*  函数参数：
*         pxTaskCode     指向任务函数的指针
*         pcName         任务名，最大长度为 configMAX_TASK_NAME_LEN
*         usStackDepth   任务堆栈大小， 单位：字（注意，单位不是字节）
*         pvParameters   传递给任务函数的参数
*         uxPriority     任务优先级，最大值为(configMAX_PRIORITIES-1)
*         pxCreatedTask  任务句柄，任务成功创建后，会返回任务句柄。任务句柄就
*                        是任务的任务控制块
*
*  函数返回值：
*         pdPASS                                  任务创建成功
*         errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY   内存不足，任务创建失败
***************************************************************************/

/************************************************************************
*                       动态与静态创建任务的区别
*
*  动态创建任务：任务的任务控制块以及任务的栈空间所需的内存，均由 FreeRTOS 
*               从 FreeRTOS 管理的堆中分配。
*  静态创建任务：任务的任务控制块以及任务的栈空间所需的内存，需要由用户分配提供。
*
*  任务删除时： 动态创建的任务的任务控制块以及任务的栈空间内存会由空闲任务会负
*              责释放；静态创建的任务的任务控制块以及任务的栈空间内存必须用户
*              后期手动自己去释放。
*************************************************************************/

/**************************************************************
*  注意：一般创建任务都是采用动态创建任务的方式，后期的学习我也是
*       全都用动态创建任务的方式来创建任务
***************************************************************/

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
        vTaskDelay(500);   /* 延时500个tick */
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
        vTaskDelay(500);   /* 延时500个tick */
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
    while (1)
    {
        printf("动态创建任务3\r\n");
        vTaskDelay(500);   /* 延时500个tick */
    }
}

