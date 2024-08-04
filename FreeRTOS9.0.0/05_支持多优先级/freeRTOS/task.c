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
	
	#if 0
	#define taskRESET_READY_PRIORITY( uxPriority )														                        \
	{																									                                                \
		if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ ( uxPriority ) ] ) ) == ( UBaseType_t ) 0 )	\
		{																								                                                \
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
}

//在main.c里定义的任务控制块
extern TCB_t Task1TCB;
extern TCB_t Task2TCB;

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

	/* 设置延时时间 */
	pxTCB->xTicksToDelay = xTicksToDelay;
	
	/* 将任务从就绪列表移除 */
	//uxListRemove( &( pxTCB->xStateListItem ) );      //由于需要用到，所以这部操作暂时不做，下一章会进行操作
	taskRESET_READY_PRIORITY( pxTCB->uxPriority );

	/* 任务切换 */
	taskYIELD();
}



//滴答定时器中断里进行任务阻塞延时更新
void xTaskIncrementTick( void )
{
	TCB_t *pxTCB = NULL;
	BaseType_t i = 0;

	/* 更新系统时基计数器xTickCount，xTickCount是一个在port.c中定义的全局变量 */
	const TickType_t xConstTickCount = xTickCount + 1;
	xTickCount = xConstTickCount;


	/* 扫描就绪列表中所有线程的xTicksToDelay，如果不为0，则减1 */
	for(i=0; i<configMAX_PRIORITIES; i++)
	{
		pxTCB = ( TCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( ( &pxReadyTasksLists[i] ) );
		if(pxTCB->xTicksToDelay > 0)
		{
			pxTCB->xTicksToDelay --;
			
			/* 延时时间到，将任务就绪 */
			if( pxTCB->xTicksToDelay ==0 )
			{
				taskRECORD_READY_PRIORITY( pxTCB->uxPriority );
			}
		}
	}

	/* 任务切换 */
	portYIELD();
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






