#include "FreeRTOS.h"
#include "task.h"

// TCB 就是任务控制块的英文缩写


/********************************全局变量********************************/

/* 当前正在运行的任务的任务控制块指针，默认初始化为NULL */
TCB_t * volatile pxCurrentTCB = NULL;

//空闲任务句柄
static TaskHandle_t xIdleTaskHandle	= NULL; //(由于只定义赋值后没有再使用，所以编译器会有一个警告，这个不用理)
//用于阻塞延时
static volatile TickType_t xTickCount = ( TickType_t ) 0U;

/* 任务就绪列表 */
//就是组着链表根节点的数组
List_t pxReadyTasksLists[ configMAX_PRIORITIES ];

static volatile UBaseType_t uxTopReadyPriority = tskIDLE_PRIORITY;          //当前最高优先级

static volatile UBaseType_t uxCurrentNumberOfTasks 	= ( UBaseType_t ) 0U;    //添加的任务数
static UBaseType_t uxTaskNumber = ( UBaseType_t ) 0U;


//任务延时列表
static List_t xDelayedTaskList1;                     //xTickCount没有溢出来时只用这个列表
static List_t xDelayedTaskList2;                     //xTickCount溢出来时只用这个列表
static List_t * volatile pxDelayedTaskList;          //指向延时列表1
static List_t * volatile pxOverflowDelayedTaskList;  //指向延时列表2

static volatile TickType_t xNextTaskUnblockTime		= ( TickType_t ) 0U;   //任务延时值
static volatile BaseType_t xNumOfOverflows 			  = ( BaseType_t ) 0;


/********************************宏定义********************************/

/* 将任务添加到就绪列表 */                                    
#define prvAddTaskToReadyList( pxTCB )												      \
	taskRECORD_READY_PRIORITY( ( pxTCB )->uxPriority );								\
	vListInsertEnd( &( pxReadyTasksLists[ ( pxTCB )->uxPriority ] ),  \
									&( ( pxTCB )->xStateListItem ) );                 \
/*查找最高优先级的就绪任务*/
//通用方法
#if ( configUSE_PORT_OPTIMISED_TASK_SELECTION == 0 )
	/* uxTopReadyPriority 存的是就绪任务的最高优先级 */
	#define taskRECORD_READY_PRIORITY( uxPriority )		\
	{																									\
		if( ( uxPriority ) > uxTopReadyPriority )				\
		{																								\
			uxTopReadyPriority = ( uxPriority );					\
		}																								\
	} /* taskRECORD_READY_PRIORITY */
/*-------------------------------------------------------------------*/
	#define taskSELECT_HIGHEST_PRIORITY_TASK()															               \
	{																									                                     \
		UBaseType_t uxTopPriority = uxTopReadyPriority;														           \
		/* 寻找包含就绪任务的最高优先级的队列 */                                               \
		/* listLIST_IS_EMPTY 判断列表是否为空 */                                              \
		while( listLIST_IS_EMPTY( &( pxReadyTasksLists[ uxTopPriority ] ) ) )							   \
		{																								                                     \
			--uxTopPriority;																			                             \
		}																								                                     \
		/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */							               \
		listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( pxReadyTasksLists[ uxTopPriority ] ) );\
		/* 更新uxTopReadyPriority */                                                         \
		uxTopReadyPriority = uxTopPriority;																                   \
	} /* taskSELECT_HIGHEST_PRIORITY_TASK */
	
	/* 这两个宏定义只有在选择优化方法时才用，这里定义为空 */
	#define taskRESET_READY_PRIORITY( uxPriority )
	#define portRESET_READY_PRIORITY( uxPriority, uxTopReadyPriority )
	
/* 查找最高优先级的就绪任务：根据处理器架构优化后的方法 */
#else
	//添加优先级
	#define taskRECORD_READY_PRIORITY( uxPriority )	portRECORD_READY_PRIORITY(uxPriority, uxTopReadyPriority)
	
	/*------------------------------------------------------------------------------*/
	//获取最高优先级任务
	#define taskSELECT_HIGHEST_PRIORITY_TASK()												\
	{																								                  \
		UBaseType_t uxTopPriority;																		  \
		/* 寻找最高优先级 */								                              \
		portGET_HIGHEST_PRIORITY( uxTopPriority, uxTopReadyPriority );	\
		/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */       \
		listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( pxReadyTasksLists[ uxTopPriority ] ) );\
	} /* taskSELECT_HIGHEST_PRIORITY_TASK() */
	
	/*------------------------------------------------------------------------------*/
	
	#if 1
	#define taskRESET_READY_PRIORITY( uxPriority )														                        \
	{																									                                                \
		/* 判断节点数是否为0 */                                                                          \
		if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ ( uxPriority ) ] ) ) == ( UBaseType_t ) 0 )	\
		{																								                                                \
			/* 恢复优先级 */                                                                               \
			portRESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );							              \
		}																								                                                \
	}
	#else
	#define taskRESET_READY_PRIORITY( uxPriority )                          \
	{																							                          \
		portRESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );	  \
	}
	#endif
#endif 


/* 延时列表切换 */
/***********************************************************
 * 当系统时基计数器溢出的时候，延时列表pxDelayedTaskList 和
 * pxOverflowDelayedTaskList要互相切换
 **************************************************************/
#define taskSWITCH_DELAYED_LISTS()\
{																								\
	List_t *pxTemp;																\
	pxTemp = pxDelayedTaskList;										\
	pxDelayedTaskList = pxOverflowDelayedTaskList;\
	pxOverflowDelayedTaskList = pxTemp;						\
	xNumOfOverflows++;														\
	prvResetNextTaskUnblockTime();								\
}



/************************************************************************/

//任务创建有两种方式
//一种是动态创建，系统动态分配任务控制块和任务栈的内存空间，任务删除后内存空间会被回收
//一种是静态创建，需要自己事先定义好任务控制块和任务栈的内存空间大小，任务删除后，内存空间依然存在，必须手动释放
/*这个代码表示选择任务创建方式，0-动态，1-静态*/
#if( configSUPPORT_STATIC_ALLOCATION == 1 )

/* 静态任务创建函数 */
/* 任务的栈， 任务的函数实体， 任务的控制块联系起来 */
TaskHandle_t xTaskCreateStatic(	TaskFunction_t pxTaskCode,           /* 任务入口 */
																const char * const pcName,           /* 任务名称，字符串形式 */
																const uint32_t ulStackDepth,         /* 任务栈大小，单位为字 */
																void * const pvParameters,           /* 任务形参 */
																UBaseType_t uxPriority,              /* 任务优先级，数值越大，优先级越高 */
																StackType_t * const puxStackBuffer,  /* 任务栈起始地址 */
																TCB_t * const pxTaskBuffer )         /* 任务控制块指针 */
{
	TCB_t *pxNewTCB;
	TaskHandle_t xReturn;   //定义一个任务句柄xReturn，任务句柄用于指向任务的TCB
	
	//判断传入的任务控制块和任务栈起始地址的变量地址是否为空
	if((pxTaskBuffer != NULL) && (puxStackBuffer != NULL))
	{
		pxNewTCB = ( TCB_t * ) pxTaskBuffer; 
		pxNewTCB->pxStack = ( StackType_t * ) puxStackBuffer;
		
		/* 创建新的任务 */
		prvInitialiseNewTask( pxTaskCode,        /* 任务入口 */
													pcName,            /* 任务名称，字符串形式 */
													ulStackDepth,      /* 任务栈大小，单位为字 */ 
													pvParameters,      /* 任务形参 */
													uxPriority,        /* 任务优先级 */
													&xReturn,          /* 任务句柄 */ 
													pxNewTCB);         /* 任务控制块 */ 
													
		/* 将任务添加到就绪列表 */
		prvAddNewTaskToReadyList( pxNewTCB );
	}
	else
	{
		xReturn = NULL;
	}

	/* 返回任务句柄，如果任务创建成功，此时xReturn应该指向任务控制块 */
	return xReturn;
}

#endif /* configSUPPORT_STATIC_ALLOCATION */


/* 创建新任务 */
static void prvInitialiseNewTask( 	TaskFunction_t pxTaskCode,              /* 任务入口 */
																		const char * const pcName,              /* 任务名称，字符串形式 */
																		const uint32_t ulStackDepth,            /* 任务栈大小，单位为字 */
																		void * const pvParameters,              /* 任务形参 */
																		UBaseType_t uxPriority,                 /* 任务优先级，数值越大，优先级越高 */
																		TaskHandle_t * const pxCreatedTask,     /* 任务句柄 */
																		TCB_t *pxNewTCB )                       /* 任务控制块指针 */
{
	StackType_t *pxTopOfStack;    //存放栈顶地址
	UBaseType_t x;	
	
	/* 获取栈顶地址 */
	//栈顶地址 = 任务起始地址 + (任务栈大小 - 1)
	pxTopOfStack = pxNewTCB->pxStack + ( ulStackDepth - ( uint32_t ) 1 );
	
	/* 向下做8字节对齐 */
	//就是将数据作出能被8整除的数
	//例子(以下都是十进制数)：64 -> 64     65 -> 64    68 -> 64     72 -> 72
	//在做向下 8 字节对齐的时候，就会空出几个字节，不会使用
	pxTopOfStack = ( StackType_t * ) ( ( ( uint32_t ) pxTopOfStack ) & ( ~( ( uint32_t ) 0x0007 ) ) );	
	
	/* 将任务的名字存储在TCB中 */
	for( x = ( UBaseType_t ) 0; x < ( UBaseType_t ) configMAX_TASK_NAME_LEN; x++ )
	{
		pxNewTCB->pcTaskName[ x ] = pcName[ x ];

		if( pcName[ x ] == 0x00 )
		{
			break;
		}
	}
	
	/* 任务名字的长度不能超过configMAX_TASK_NAME_LEN */
	pxNewTCB->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';
	
	/* 初始化TCB中的xStateListItem任务节点 */
	//链表数据节点初始化
  vListInitialiseItem( &( pxNewTCB->xStateListItem ) );
	
	/* 设置xStateListItem节点的拥有者 */
	//链表数据节点的任务控制块拥有者
	listSET_LIST_ITEM_OWNER( &( pxNewTCB->xStateListItem ), pxNewTCB );
	
	/* 初始化优先级 */
	if( uxPriority >= ( UBaseType_t ) configMAX_PRIORITIES )
	{
		uxPriority = ( UBaseType_t ) configMAX_PRIORITIES - ( UBaseType_t ) 1U;
	}
	pxNewTCB->uxPriority = uxPriority;
	
	/* 初始化任务栈 */
	pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack, pxTaskCode, pvParameters );
	
	/* 让任务句柄指向任务控制块 */
  if( ( void * ) pxCreatedTask != NULL )
	{		
		*pxCreatedTask = ( TaskHandle_t ) pxNewTCB;
	}
}



/* 就绪列表初始化 */
//其实就是对数组成员进行根节点初始化
void prvInitialiseTaskLists( void )
{
	UBaseType_t uxPriority;
     
	for(uxPriority = ( UBaseType_t ) 0U;
			uxPriority < ( UBaseType_t ) configMAX_PRIORITIES;
			uxPriority++ )
	{
		//根节点初始化
		vListInitialise( &( pxReadyTasksLists[ uxPriority ] ) );
	}
	
	//初始化延时列表1
	vListInitialise( &xDelayedTaskList1 );
	pxDelayedTaskList = &xDelayedTaskList1;
	//初始化延时列表2
	vListInitialise( &xDelayedTaskList2 );
	pxOverflowDelayedTaskList = &xDelayedTaskList2;
}

//空闲任务
extern TCB_t IdleTaskTCB;
void vApplicationGetIdleTaskMemory( TCB_t **ppxIdleTaskTCBBuffer, 
                                    StackType_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize );

/*开启任务调度器*/
void vTaskStartScheduler( void )
{
	/*======================================创建空闲任务start==============================================*/     
	TCB_t *pxIdleTaskTCBBuffer = NULL;               /* 用于指向空闲任务控制块 */
	StackType_t *pxIdleTaskStackBuffer = NULL;       /* 用于空闲任务栈起始地址 */
	uint32_t ulIdleTaskStackSize;                    /* 用于设置空闲任务栈大小 */

	/* 获取空闲任务的内存：任务栈和任务TCB */
	vApplicationGetIdleTaskMemory( &pxIdleTaskTCBBuffer, 
																 &pxIdleTaskStackBuffer, 
																 &ulIdleTaskStackSize );    

	xIdleTaskHandle = xTaskCreateStatic( (TaskFunction_t)prvIdleTask,              /* 任务入口 */
																			 (char *)"IDLE",                           /* 任务名称，字符串形式 */
																			 (uint32_t)ulIdleTaskStackSize ,           /* 任务栈大小，单位为字 */
																			 (void *) NULL,                            /* 任务形参 */
																			 (UBaseType_t) tskIDLE_PRIORITY,           /* 任务优先级，数值越大，优先级越高 */
																			 (StackType_t *)pxIdleTaskStackBuffer,     /* 任务栈起始地址 */
																			 (TCB_t *)pxIdleTaskTCBBuffer );           /* 任务控制块 */
	/*======================================创建空闲任务end================================================*/
		
		xNextTaskUnblockTime = portMAX_DELAY;
    xTickCount = ( TickType_t ) 0U;
						
	/* 启动调度器 */
	if( xPortStartScheduler() != pdFALSE )
	{
			/* 调度器启动成功，则不会返回，即不会来到这里 */
	}
}

//空闲任务
static portTASK_FUNCTION( prvIdleTask, pvParameters )
{
	/* 防止编译器的警告 */
	( void ) pvParameters;
    
    for(;;)
    {
        /* 空闲任务暂时什么都不做 */
    }
}

//进行任务切换
void vTaskDelay( const TickType_t xTicksToDelay )
{
	TCB_t *pxTCB = NULL;

	/* 获取当前任务的TCB */
	pxTCB = pxCurrentTCB;
	
	/* 将任务插入到延时列表 */
	prvAddCurrentTaskToDelayedList( xTicksToDelay );

	/* 任务切换 */
	taskYIELD();
}



//滴答定时器中断里进行任务阻塞延时更新
BaseType_t xTaskIncrementTick( void )
{
	TCB_t *pxTCB = NULL;
	TickType_t xItemValue;
	BaseType_t xSwitchRequired = pdFALSE;

	/* 更新系统时基计数器xTickCount，xTickCount是一个在port.c中定义的全局变量 */
	const TickType_t xConstTickCount = xTickCount + 1;
	xTickCount = xConstTickCount;
	
	/* 如果xConstTickCount溢出，则切换延时列表 */
	if( xConstTickCount == ( TickType_t ) 0U )
	{
		//切换延时列表
		taskSWITCH_DELAYED_LISTS();
	}

	/* 最近的延时任务延时到期 */
	if( xConstTickCount >= xNextTaskUnblockTime )
	{
		for( ;; )
		{	
			//判断延时列表是否为空
			//listLIST_IS_EMPTY判断判断延时列表是否为空，空则返回1，非空则返回1
			if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
			{
				/* 延时列表为空，设置xNextTaskUnblockTime为可能的最大值 */
				xNextTaskUnblockTime = portMAX_DELAY;
				break;
			}
			//延时列表不为空
			else
			{
				//获取延时列表的第一个任务控制块TCB
				pxTCB = ( TCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList );
				//获取任务的辅助值
				xItemValue = listGET_LIST_ITEM_VALUE( &( pxTCB->xStateListItem ) );
				
				/* 直到将延时列表中所有延时到期的任务移除才跳出for循环 */
				if( xConstTickCount < xItemValue )
				{
					xNextTaskUnblockTime = xItemValue;
					break;
				}
				
				/* 将任务从延时列表移除，消除等待状态 */
				( void ) uxListRemove( &( pxTCB->xStateListItem ) );

				/* 将解除等待的任务添加到就绪列表 */
				prvAddTaskToReadyList( pxTCB );
				
				#if ( configUSE_PREEMPTION == 1 )
				{
					if( pxTCB->uxPriority >= pxCurrentTCB->uxPriority )
					{
						xSwitchRequired = pdTRUE;
					}
				}
				#endif /* configUSE_PREEMPTION */
			}
		}
	}
	
	#if ( ( configUSE_PREEMPTION == 1 ) && ( configUSE_TIME_SLICING == 1 ) )
	{
		if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ pxCurrentTCB->uxPriority ] ) ) 
				> ( UBaseType_t ) 1 )
		{
			xSwitchRequired = pdTRUE;
		}
	}
	#endif /* ( ( configUSE_PREEMPTION == 1 ) && ( configUSE_TIME_SLICING == 1 ) ) */
	
	return xSwitchRequired;
}



/************************** 最高优先级查询---修改全局控制块，实现任务切换 *****************************/
/* 任务切换，即寻找优先级最高的就绪任务 */
void vTaskSwitchContext( void )
{
	/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */
    taskSELECT_HIGHEST_PRIORITY_TASK();
}



//将任务添加到就绪列表
static void prvAddNewTaskToReadyList( TCB_t *pxNewTCB )
{
	/* 进入临界段 */
	taskENTER_CRITICAL();
	{
		/* 全局任务计时器加一操作 */
		uxCurrentNumberOfTasks++;
        
		/* 如果pxCurrentTCB为空，则将pxCurrentTCB指向新创建的任务 */
		if( pxCurrentTCB == NULL )
		{
			pxCurrentTCB = pxNewTCB;

			/* 如果是第一次创建任务，则需要初始化任务相关的列表 */
			if( uxCurrentNumberOfTasks == ( UBaseType_t ) 1 )
			{
				/* 初始化任务相关的列表 */
				prvInitialiseTaskLists();
			}
		}
		else /* 如果pxCurrentTCB不为空，则根据任务的优先级将pxCurrentTCB指向最高优先级任务的TCB */
		{
			if( pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority )
			{
				pxCurrentTCB = pxNewTCB;
			}
		}
		uxTaskNumber++;
        
		/* 将任务添加到就绪列表 */
    prvAddTaskToReadyList( pxNewTCB );

	}
	/* 退出临界段 */
	taskEXIT_CRITICAL();
}


//将要延时的任务插入延时列表
static void prvAddCurrentTaskToDelayedList( TickType_t xTicksToWait )
{
	TickType_t xTimeToWake;

	/* 获取系统时基计数器xTickCount的值 */
	const TickType_t xConstTickCount = xTickCount;

	/* 将任务从就绪列表中移除 */
	if( uxListRemove( &( pxCurrentTCB->xStateListItem ) ) == ( UBaseType_t ) 0 )
	{
		/* 将任务在优先级位图中对应的位清除 */
		portRESET_READY_PRIORITY( pxCurrentTCB->uxPriority, uxTopReadyPriority );
	}

	/* 计算延时到期时，系统时基计数器xTickCount的值是多少 */
	//延时到期时间 = 当前系统时基 + 要延时的时长
	xTimeToWake = xConstTickCount + xTicksToWait;

	/* 将延时到期的值设置为节点的排序值 */
	listSET_LIST_ITEM_VALUE( &( pxCurrentTCB->xStateListItem ), xTimeToWake );

	/* 溢出 */
	if( xTimeToWake < xConstTickCount )
	{
		//溢出的放在延时列表2中
		vListInsert( pxOverflowDelayedTaskList, &( pxCurrentTCB->xStateListItem ) );
	}
	else /* 没有溢出 */
	{
		//没溢出的放在延时列表1中
		vListInsert( pxDelayedTaskList, &( pxCurrentTCB->xStateListItem ) );

		/* 更新下一个任务解锁时刻变量xNextTaskUnblockTime的值 */
		if( xTimeToWake < xNextTaskUnblockTime )
		{
			xNextTaskUnblockTime = xTimeToWake;
		}
	}	
}


static void prvResetNextTaskUnblockTime( void )
{
	TCB_t *pxTCB;

	if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
	{
		/* 当前延时列表为空，则设置 xNextTaskUnblockTime 等于最大值 */
		xNextTaskUnblockTime = portMAX_DELAY;
	}
	else
	{
		/* 当前列表不为空，则有任务在延时，则获取当前列表下第一个节点的排序值
			 然后将该节点的排序值更新到 xNextTaskUnblockTime*/
		( pxTCB ) = ( TCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList );
		xNextTaskUnblockTime = listGET_LIST_ITEM_VALUE( &( ( pxTCB )->xStateListItem ) );
	}
}



