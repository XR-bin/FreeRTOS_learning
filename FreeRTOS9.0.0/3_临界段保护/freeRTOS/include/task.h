#ifndef TASK_H
#define TASK_H

	#include "list.h"
	

	/* 任务句柄 */
	typedef void * TaskHandle_t;
	#define taskYIELD()			portYIELD()      //任务切换
	
	
	/***************************************************************************
	*                               变量声明                                   *
	****************************************************************************/
	extern List_t pxReadyTasksLists[ configMAX_PRIORITIES ];
	
	/***************************************************************************
	*                               函数声明                                   *
	****************************************************************************/
	static void prvInitialiseNewTask( 	TaskFunction_t pxTaskCode,              /* 任务入口 */
																			const char * const pcName,              /* 任务名称，字符串形式 */
																			const uint32_t ulStackDepth,            /* 任务栈大小，单位为字 */
																			void * const pvParameters,              /* 任务形参 */
																			TaskHandle_t * const pxCreatedTask,     /* 任务句柄 */
																			TCB_t *pxNewTCB );                      /* 任务控制块指针 */
	
	#if( configSUPPORT_STATIC_ALLOCATION == 1 )
	TaskHandle_t xTaskCreateStatic(	TaskFunction_t pxTaskCode,           /* 任务入口 */
																	const char * const pcName,           /* 任务名称，字符串形式 */
																	const uint32_t ulStackDepth,         /* 任务栈大小，单位为字 */
																	void * const pvParameters,           /* 任务形参 */
																	StackType_t * const puxStackBuffer,  /* 任务栈起始地址 */
																	TCB_t * const pxTaskBuffer );         /* 任务控制块指针 */	
	#endif /* configSUPPORT_STATIC_ALLOCATION */
	
	void prvInitialiseTaskLists( void );
	void vTaskStartScheduler( void );
	void vTaskSwitchContext( void );

#endif
