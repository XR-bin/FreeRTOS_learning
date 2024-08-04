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

#endif /* PORTMACRO_H */


