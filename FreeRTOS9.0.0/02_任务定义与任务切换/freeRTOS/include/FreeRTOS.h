#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H

	#include "FreeRTOSConfig.h"
	#include "projdefs.h"
	#include "portable.h"
	#include "list.h"


	//任务控制块(TCB)结构体
	typedef struct tskTaskControlBlock
	{
		volatile StackType_t    *pxTopOfStack;                            //栈顶指针
		ListItem_t			        xStateListItem;                           //任务节点
		StackType_t             *pxStack;                                 //任务栈起始地址
		char                    pcTaskName[ configMAX_TASK_NAME_LEN ];    //任务名称，字符串形式
	} tskTCB;
	typedef tskTCB TCB_t;


#endif /* INC_FREERTOS_H */
