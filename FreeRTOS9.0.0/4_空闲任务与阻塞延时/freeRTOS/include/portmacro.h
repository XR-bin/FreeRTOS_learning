#ifndef PORTMACRO_H
#define PORTMACRO_H

#include "stdint.h"
#include "stddef.h"


/* 数据类型重定义 */
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		long
#define portSHORT		short
#define portSTACK_TYPE	uint32_t
#define portBASE_TYPE	long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
	typedef uint16_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffff
#else
	typedef uint32_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif


/****************************************
*  中断控制状态寄存器：0xe000ed04
*  Bit 28 PENDSVSET: PendSV 悬起位
******************************************/
#define portNVIC_INT_CTRL_REG		( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT		( 1UL << 28UL )

#define portSY_FULL_READ_WRITE		( 15 )

//任务切换
/********************************************************
*ISB：指令同步隔离，最严格，所有它前面的指令都执行完毕之后，才执行后面的指令
*DSB：数据同步隔离，比DMB严格，仅当前所有在它面前的存储器访问操作都执行完毕后，才执行在它后面的指令(亦是即任何指令都要等待存储器访问操作完成)
*DMB：数据存储隔离，DMS指令保证仅当前所有在它前面的存储器访问操作都执行完毕后，才提交(commit)在它后面存储器访问操作
*********************************************************/
#define portYIELD()																\
{																				          \
	/* 触发PendSV中断，产生上下文切换 */							\
	portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;	\
	/* 防止多次进入中断 */                           \
	__dsb( portSY_FULL_READ_WRITE );								\
	__isb( portSY_FULL_READ_WRITE );								\
}


/*********************************************************
*								        临界保护
**********************************************************/

#define portDISABLE_INTERRUPTS()				vPortRaiseBASEPRI()    //关中断(不可嵌套中断里)
#define portENABLE_INTERRUPTS()					vPortSetBASEPRI( 0 )   //开中断

#define portENTER_CRITICAL()					vPortEnterCritical()
#define portEXIT_CRITICAL()						vPortExitCritical()

//【注意】一般函数名/宏定义的末尾带有FROM_ISR，都表示可以用在中断里
#define portSET_INTERRUPT_MASK_FROM_ISR()		ulPortRaiseBASEPRI()    //关中断(可嵌套在中断里)
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)	vPortSetBASEPRI(x)    //选择性开中断

#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )  //用来定义一个函数

#define portINLINE __inline

#ifndef portFORCE_INLINE
	#define portFORCE_INLINE __forceinline
#endif

//不带返回值的关中断函数(根据参数设置)------不能嵌套，不能在中断里面使用
static portFORCE_INLINE void vPortSetBASEPRI( uint32_t ulBASEPRI )
{
	__asm
	{
		//直接设置中断屏蔽值，大于ulBASEPRI值的中断无法触发
		msr basepri, ulBASEPRI
	}
}
//不带返回值的关中断函数(根据configMAX_SYSCALL_INTERRUPT_PRIORITY宏定义设置)------不能嵌套，不能在中断里面使用
static portFORCE_INLINE void vPortRaiseBASEPRI( void )
{
	uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		/* Set BASEPRI to the max syscall priority to effect a critical
		section. */
		msr basepri, ulNewBASEPRI
		dsb
		isb
	}
}

//不带返回值的开中断函数
static portFORCE_INLINE void vPortClearBASEPRIFromISR( void )
{
	__asm
	{
		/* Set BASEPRI to 0 so no interrupts are masked.  This function is only
		used to lower the mask in an interrupt, so memory barriers are not 
		used. */
		msr basepri, #0
	}
}

//带返回值的关中断函数，可以嵌套，可以在中断里面使用
static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI( void )
{
	uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		/* Set BASEPRI to the max syscall priority to effect a critical
		section. */
		mrs ulReturn, basepri
		msr basepri, ulNewBASEPRI
		dsb
		isb
	}

	return ulReturn;
}


#endif /* PORTMACRO_H */


