#include "FreeRTOS.h"
#include "task.h"

// TCB 就是任务控制块的英文缩写


/********************************全局变量********************************/

/* 当前正在运行的任务的任务控制块指针，默认初始化为NULL */
TCB_t * volatile pxCurrentTCB = NULL;

/* 任务就绪列表 */
//就是组着链表根节点的数组
List_t pxReadyTasksLists[ configMAX_PRIORITIES ];

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
													&xReturn,          /* 任务句柄 */ 
													pxNewTCB);         /* 任务控制块 */  
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
/*开启任务调度器*/
void vTaskStartScheduler( void )
{
    /* 手动指定第一个运行的任务 */
    pxCurrentTCB = &Task1TCB;
    
    /* 启动调度器 */
    if( xPortStartScheduler() != pdFALSE )
    {
        /* 调度器启动成功，则不会返回，即不会来到这里 */
    }
}

//修改控制块，实现任务切换
void vTaskSwitchContext( void )
{    
    /* 两个任务轮流切换 */
    if( pxCurrentTCB == &Task1TCB )
    {
        pxCurrentTCB = &Task2TCB;
    }
    else
    {
        pxCurrentTCB = &Task1TCB;
    }
}



