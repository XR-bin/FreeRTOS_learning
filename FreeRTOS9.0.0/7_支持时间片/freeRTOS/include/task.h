#ifndef TASK_H
#define TASK_H

	#include "list.h"
	

	/* 任务句柄 */
	typedef void * TaskHandle_t;
	#define taskYIELD()			portYIELD()      //任务切换
	#define tskIDLE_PRIORITY			       ( ( UBaseType_t ) 0U )
	
	//进出临界面
	#define taskENTER_CRITICAL()		        portENTER_CRITICAL()
	#define taskENTER_CRITICAL_FROM_ISR()   portSET_INTERRUPT_MASK_FROM_ISR()

	#define taskEXIT_CRITICAL()			        portEXIT_CRITICAL()
	#define taskEXIT_CRITICAL_FROM_ISR( x ) portCLEAR_INTERRUPT_MASK_FROM_ISR( x )
	
	
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
																			UBaseType_t uxPriority,                 /* 任务优先级，数值越大，优先级越高 */
																			TaskHandle_t * const pxCreatedTask,     /* 任务句柄 */
																			TCB_t *pxNewTCB );                      /* 任务控制块指针 */
	
	#if( configSUPPORT_STATIC_ALLOCATION == 1 )
	TaskHandle_t xTaskCreateStatic(	TaskFunction_t pxTaskCode,           /* 任务入口 */
																	const char * const pcName,           /* 任务名称，字符串形式 */
																	const uint32_t ulStackDepth,         /* 任务栈大小，单位为字 */
																	void * const pvParameters,           /* 任务形参 */
																	UBaseType_t uxPriority,              /* 任务优先级，数值越大，优先级越高 */
																	StackType_t * const puxStackBuffer,  /* 任务栈起始地址 */
																	TCB_t * const pxTaskBuffer );         /* 任务控制块指针 */	
	#endif /* configSUPPORT_STATIC_ALLOCATION */
	
	void prvInitialiseTaskLists( void );
	void vTaskStartScheduler( void );
	void vTaskSwitchContext( void );
	static portTASK_FUNCTION( prvIdleTask, pvParameters );
	void vTaskDelay( const TickType_t xTicksToDelay );
	BaseType_t xTaskIncrementTick( void );
	static void prvAddNewTaskToReadyList( TCB_t *pxNewTCB );
	static void prvAddCurrentTaskToDelayedList( TickType_t xTicksToWait );
	static void prvResetNextTaskUnblockTime( void );

#endif
