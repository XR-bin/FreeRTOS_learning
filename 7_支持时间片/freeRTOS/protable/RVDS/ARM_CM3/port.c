#include "FreeRTOS.h"
#include "task.h"
#include "ARMCM3.h"

#define portINITIAL_XPSR			        ( 0x01000000 )
#define portSTART_ADDRESS_MASK				( ( StackType_t ) 0xfffffffeUL )

static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

/* 
 * 参考资料《STM32F10xxx Cortex-M3 programming manual》4.4.3，百度搜索“PM0056”即可找到这个文档
 * 在Cortex-M中，内核外设SCB中SHPR3寄存器用于设置SysTick和PendSV的异常优先级
 * System handler priority register 3 (SCB_SHPR3) SCB_SHPR3：0xE000 ED20
 * Bits 31:24 PRI_15[7:0]: Priority of system handler 15, SysTick exception 
 * Bits 23:16 PRI_14[7:0]: Priority of system handler 14, PendSV 
 */

//SHPR3寄存器：系统处理程序优先级寄存器3
#define portNVIC_SYSPRI2_REG				( * ( ( volatile uint32_t * ) 0xe000ed20 ) )

//【注意】设置值越大，优先级越低
//可挂起系统的系统服务中断优先级
#define portNVIC_PENDSV_PRI					( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )
//滴答定时器中断优先级
#define portNVIC_SYSTICK_PRI				( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )


/* SysTick 配置寄存器 */
#define portNVIC_SYSTICK_CTRL_REG			( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG			( * ( ( volatile uint32_t * ) 0xe000e014 ) )

#ifndef configSYSTICK_CLOCK_HZ
	#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
	/* 确保SysTick的时钟与内核时钟一致 */
	#define portNVIC_SYSTICK_CLK_BIT	( 1UL << 2UL )
#else
	#define portNVIC_SYSTICK_CLK_BIT	( 0 )
#endif

#define portNVIC_SYSTICK_INT_BIT			  ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT			( 1UL << 0UL )


/*************************************************************************
*                              函数声明
*************************************************************************/
void prvStartFirstTask(void); 
void vPortSVCHandler(void);
void vPortSetupTimerInterrupt(void);




/*************************************************************************
*                              任务栈初始化函数
*************************************************************************/

static void prvTaskExitError( void )
{
    /* 函数停止在这里 */
    while(1);
}

StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
	//pxTopOfStack是任务栈顶地址
	//pxCode是任务入口函数
	//pvParameters任务形参
	
	/* 异常发生时，自动加载到CPU寄存器的内容 */
	pxTopOfStack--;
	*pxTopOfStack = portINITIAL_XPSR;	                                    /* xPSR的bit24必须置1 */
	pxTopOfStack--;
	*pxTopOfStack = ( ( StackType_t ) pxCode ) & portSTART_ADDRESS_MASK;	/* PC，即任务入口函数 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) prvTaskExitError;	                    /* LR，函数返回地址 */
	pxTopOfStack -= 5;	/* R12, R3, R2 and R1 默认初始化为0 */
	*pxTopOfStack = ( StackType_t ) pvParameters;	                        /* R0，任务形参 */

	/* 异常发生时，手动加载到CPU寄存器的内容 */    
	pxTopOfStack -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4默认初始化为0 */

	/* 返回栈顶指针，此时pxTopOfStack指向空闲栈 */
	return pxTopOfStack;
}



/*************************************************************************
*                           任务调度器启动函数
*************************************************************************/
BaseType_t xPortStartScheduler(void)
{
	/* 配置PendSV 和 SysTick 的中断优先级为最低 */
	//SHPR3寄存器：系统处理程序优先级寄存器3
	//PendSV：可挂起系统的系统服务
	//SysTick：滴答定时器
	portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
	portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;
	
	/* 初始化SysTick */
	vPortSetupTimerInterrupt();

	/* 启动第一个任务，不再返回 */
	prvStartFirstTask();

	/* 不应该运行到这里 */
	return 0;
}

/*
 * 参考资料《STM32F10xxx Cortex-M3 programming manual》4.4.3，百度搜索“PM0056”即可找到这个文档
 * 在Cortex-M中，内核外设SCB的地址范围为：0xE000ED00-0xE000ED3F
 * 0xE000ED008为SCB外设中SCB_VTOR这个寄存器的地址，里面存放的是向量表的起始地址，即MSP的地址
 */
__asm void prvStartFirstTask( void )
{
	/*PRESERVE8指令指定当前文件保持堆栈八字节对齐*/
	PRESERVE8
	

	/* 在Cortex-M中，0xE000ED08是SCB_VTOR这个寄存器的地址，
       里面存放的是向量表的起始地址，即MSP的地址 */
	ldr r0, =0xE000ED08        //把0xE000ED08地址存到r0
	ldr r0, [r0]               //通过0xE000ED08地址得到寄存器的值(SCB_VTOR存的值)
	ldr r0, [r0]               //通过SCB_VTOR存的值(这个值是矢量表偏移地址)获得MSP的地址

	/* 设置主堆栈指针msp的值 */
	msr msp, r0                //把r0的值赋值给msp寄存器
    
	/* 使能全局中断 */
	/*************************************************
	*中断与异常开关指令：
	*				cpsie i         开启中断(PRIMASK=0)
	*       cpsid i         关闭中断(PRIMASK=1)
	*       cpsie f         开启异常(FAULTMASK=0)
	*       cpsid f         关闭异常(FAULTMASK=1)
	***************************************************/
	cpsie i
	cpsie f
	//隔离指令
	/********************************************************
	*ISB：指令同步隔离，最严格，所有它前面的指令都执行完毕之后，才执行后面的指令
	*DSB：数据同步隔离，比DMB严格，仅当前所有在它面前的存储器访问操作都执行完毕后，才执行在它后面的指令(亦是即任何指令都要等待存储器访问操作完成)
	*DMB：数据存储隔离，DMS指令保证仅当前所有在它前面的存储器访问操作都执行完毕后，才提交(commit)在它后面存储器访问操作
	*********************************************************/
	dsb
	isb

	/* 调用SVC去启动第一个任务 */
	/*********************************
	* 调用SVC的0号中断服务
	*      svc_handler    0号服务
	*      svc_handler_1  1号服务
	**********************************/
	svc 0 
	/* nop 空操作，起到延时作用 */
	nop
	nop
}


//SVC中断服务
/**************************************************************************
*     SVC 中断要想被成功响应，其函数名必须与向量表注册的名称一致，在启动文件的向
*量表中， SVC 的中断服务函数注册的名称是 SVC_Handler。
*【注意】因此我们在FreeRTOSConfing.h中通过宏定义将vPortSVCHandler改为SVC_Handler
**************************************************************************/
__asm void vPortSVCHandler( void )
{
	extern pxCurrentTCB;   //引用外部定义好的任务控制块
	
	/*PRESERVE8指令指定当前文件保持堆栈八字节对齐*/
	PRESERVE8

	ldr	r3, =pxCurrentTCB	   /* 加载pxCurrentTCB的地址到r3 */
	ldr r1, [r3]			       /* 加载pxCurrentTCB地址存放的值到r1 */
	ldr r0, [r1]			       /* 加载pxCurrentTCB地址存放的值指向的值到r0，目前r0的值等于第一个任务堆栈的栈顶 */
	
	/*********************************************************************************************
	*	LDMIA 中的 I 是 increase 的缩写，A 是 after 的缩写，LD加载(load)的意思
	*	R0后面的感叹号“！表示会自动调节 R0里面的指针
	*	整句话意思是任务栈R0的存储地址由低到高，将R0存储地址里面的内容手动加载到 CPU 寄存器 R0,R4-R12里
	**********************************************************************************************/
	ldmia r0!, {r4-r11}		   /* 以r0为基地址，将栈里面的内容加载到r4~r11寄存器，同时r0会递增 */
	
	msr psp, r0				/* 将r0的值，即任务的栈指针更新到psp */
	
	//ISB：指令同步隔离，最严格，所有它前面的指令都执行完毕之后，才执行后面的指令
	isb
	
	mov r0, #0              /* 设置r0的值为0 */
	
	/*****************************************************************
	*	   设置 basepri 寄存器的值为 0，即打开所有中断。 basepri 是一个中
  * 断屏蔽寄存器，大于等于此寄存器值的中断都将被屏蔽
	******************************************************************/
	msr	basepri, r0         /* 设置basepri寄存器的值为0，即所有的中断都没有被屏蔽 */
	
	orr r14, #0xd           /* 当从SVC中断服务退出前,通过向r14寄存器最后4位按位或上0x0D，
                             使得硬件在退出时使用进程堆栈指针PSP完成出栈操作并返回后进入线程模式、返回Thumb状态 */
    
	bx r14                  /* 异常返回，这个时候栈中的剩下内容将会自动加载到CPU寄存器：
														 xPSR，PC（任务入口地址），R14，R12，R3，R2，R1，R0（任务的形参）
														 同时PSP的值也将更新，即指向任务栈的栈顶 */
}



//PendSV(挂起异常)中断服务函数
__asm void xPortPendSVHandler( void )
{
	extern pxCurrentTCB;
	extern vTaskSwitchContext;
	
	/*PRESERVE8指令指定当前文件保持堆栈八字节对齐*/
	PRESERVE8
	
	/* 当进入PendSVC Handler时，上一个任务运行的环境即：
		 xPSR，PC（任务入口地址），R14，R12，R3，R2，R1，R0（任务的形参）
		 这些CPU寄存器的值会自动保存到任务的栈中，剩下的r4~r11需要手动保存 */
	
	/* 获取任务栈指针到r0 */
	mrs r0, psp
	
	//ISB：指令同步隔离，最严格，所有它前面的指令都执行完毕之后，才执行后面的指令
	isb
	
	
	ldr	r3, =pxCurrentTCB		/* 加载旧的任务快pxCurrentTCB的到r3 */
	ldr	r2, [r3]            /* 通过r3将旧的任务快pxCurrentTCB地址到r2 */
	stmdb r0!, {r4-r11}			/* 将CPU寄存器r4~r11的值存储到r0指向的地址 */
	
	str r0, [r2]            /* 通过r2将旧的任务快pxCurrentTCB的第一个成员(旧任务的栈顶地址)，加载到r0 */				

	stmdb sp!, {r3, r14}     /* 将R3和R14临时压入堆栈，因为即将调用函数vTaskSwitchContext,
                              调用函数时,返回地址自动保存到R14中,所以一旦调用发生,R14的值会被覆盖,因此需要入栈保护;
                               R3保存的当前(旧任务)激活的任务TCB指针(pxCurrentTCB)地址,函数调用后会用到,因此也要入栈保护 */

	mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY    /* 进入临界段 */
	msr basepri, r0          //屏蔽中断
	dsb
	isb
	
	bl vTaskSwitchContext       /* 调用函数vTaskSwitchContext，寻找新的任务运行,通过使变量pxCurrentTCB指向新的任务来实现任务切换 */ 
	mov r0, #0                  /* 退出临界段 */
	msr basepri, r0             //取消中断屏蔽
	
	ldmia sp!, {r3, r14}        /* 恢复r3和r14 */
	
	//经过任务切换后，任务快pxCurrentTCB指向了新任务块，所以r3也指向新任务块
	ldr r1, [r3]
	ldr r0, [r1] 				/* 当前激活的任务TCB第一项保存了任务堆栈的栈顶,现在栈顶值存入R0*/
	ldmia r0!, {r4-r11}			/* 出栈 */
	msr psp, r0
	isb
	
	bx r14                      /* 异常发生时,R14中保存异常返回标志,包括返回后进入线程模式还是处理器模式、
																 使用PSP堆栈指针还是MSP堆栈指针，当调用 bx r14指令后，硬件会知道要从异常返回，
																 然后出栈，这个时候堆栈指针PSP已经指向了新任务堆栈的正确位置，
																 当新任务的运行地址被出栈到PC寄存器后，新的任务也会被执行。*/

	nop
}


/*************************************************************************
*                             临界段相关函数
*************************************************************************/
void vPortEnterCritical( void )
{
	portDISABLE_INTERRUPTS();
	uxCriticalNesting++;

	/* This is not the interrupt safe version of the enter critical function so
	assert() if it is being called from an interrupt context.  Only API
	functions that end in "FromISR" can be used in an interrupt.  Only assert if
	the critical nesting count is 1 to protect against recursive calls if the
	assert function also uses a critical section. */
	if( uxCriticalNesting == 1 )
	{
		//configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );
	}
}

void vPortExitCritical( void )
{
	//configASSERT( uxCriticalNesting );
	uxCriticalNesting--;
    
	if( uxCriticalNesting == 0 )
	{
		portENABLE_INTERRUPTS();
	}
}



/************************************************************************
*                             初始化SysTick
*************************************************************************/
void vPortSetupTimerInterrupt( void )
{
	/* 设置重装载寄存器的值 */
	portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;

	/* 设置系统定时器的时钟等于内核时钟
	使能SysTick 定时器中断
	使能SysTick 定时器 */
	portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT | 
																portNVIC_SYSTICK_INT_BIT |
																portNVIC_SYSTICK_ENABLE_BIT ); 
}

//滴答定时器中断服务函数
void xPortSysTickHandler( void )
{
	/* 关中断 */
	vPortRaiseBASEPRI();  
		
	{
		/* 更新系统时基 */
		if( xTaskIncrementTick() != pdFALSE )
		{
			taskYIELD();
		}
	}

	/* 开中断 */
	vPortClearBASEPRIFromISR();
}

