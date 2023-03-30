#include "FreeRTOS_demo.h"
#include "key.h"

/*****************************************************************************************
*                              列表介绍
*
*      列表是 FreeRTOS 中最基本的一种数据结构，其在物理存储单元
*  上是非连续、非顺序的(列表的本质就是链表)。列表在 FreeRTOS 中
*  的应用十分广泛，要注意的是， FreeRTOS 中的列表是一个双向链表，
*  在list.h 文件中，有列表的相关定义。
*
*  代码原型：
*          typedef struct xLIST
*          {
*               listFIRST_LIST_INTEGRITY_CHECK_VALUE                校验值 
*               volatile UBaseType_t              uxNumberOfItems;  列表中列表项的数量 
*               ListItem_t * configLIST_VOLATILE  pxIndex;          用于遍历列表 
*               MiniListItem_t                    xListEnd;         最后一个列表项(迷你列表项)
*               listSECOND_LIST_INTEGRITY_CHECK_VALUE               校验值
*          } List_t;
*
*  成员解析：
*           listFIRST_LIST_INTEGRITY_CHECK_VALUE 和 listSECOND_LIST_INTEGRITY_CHECK_VALUE ：
*           这两个宏用于存放确定已知常量， FreeRTOS通过检查这两个常量的值，来判断列表的数据在
*           程序运行过程中，是否遭到破坏，类似这样的宏定义在列表项和迷你列表项中也有出现。该功
*           能一般用于调试， 默认是不开启的，因此本教程暂不讨论这个功能。
*
*           uxNumberOfItems：用于记录列表中列表项的个数（不包含 xListEnd），当往列表中插入列
*                            表项时，该值加 1；当从列表中移除列表项时，该值减 1。
*
*           pxIndex        ：用于指向列表中的某个列表项，一般用于遍历列表中的所有列表项。(默认
*                            是指向迷你列表项)
*
*           xListEnd       ：迷你列表项。列表中迷你列表项的值一般被设置为最大值，用于将列表中
*                            的所有列表项按升序排序时，排在最末尾；同时 xListEnd 也用于挂载其
*                            他插入到列表中的列表项。
*
**********************************************************************************************/

/****************************************************************************************************
*                             列表项
*
*      列表项是列表中用于存放数据的地方，在 list.h 文件中，有列表项的
*  相关定义
*
*  代码原型：
*           struct xLIST_ITEM
*           {
*               listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE             用于检测列表项的数据完整性 
*               configLIST_VOLATILE TickType_t           xItemValue;  列表项的值
*               struct xLIST_ITEM * configLIST_VOLATILE  pxNext;      下一个列表项 
*               struct xLIST_ITEM * configLIST_VOLATILE  pxPrevious;  上一个列表项
*               void *                                   pvOwner;     列表项的拥有者
*               struct xLIST * configLIST_VOLATILE       pxContainer; 列表项所在列表
*               listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE            用于检测列表项的数据完整性
*           };
*           typedef struct xLIST_ITEM ListItem_t;
*
*  成员解析：
*          listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE 和 listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE ：
*          如同列表一样，列表项中也包含了两个用于检测列表项数据完整性的宏定义。
*          
*          xItemValue           ：列表项的值，这个值多用于按升序对列表中的列表项进行排序
*
*          pxNext 和 pxPrevious ：分别用于指向列表中列表项的下一个列表项和上一个列表项分别用于指向列表中
*                                 列表项的下一个列表项和上一个列表项。
*
*          pvOwner              ：用于指向包含列表项的对象（通常是任务控制块），因此，列表项和包含列表项
*                                 的对象之间存在双向链接。
*
*          pxContainer          ：用于指向列表项所在列表。
******************************************************************************************************/

/***************************************************************************************************
*                              迷你列表
*      迷你列表项也是列表项，但迷你列表项仅用于标记列表的末尾和挂载其他插入列表中的列表项，用户是用不
*  到迷你列表项的，在 list.h 文件中，有迷你列表项的相关定义。
*
*  代码原型：
*           struct xMINI_LIST_ITEM
*           {
*               listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE             用于检测列表项的数据完整性  
*               configLIST_VOLATILE TickType_t           xItemValue;  列表项的值 
*               struct xLIST_ITEM * configLIST_VOLATILE  pxNext;      下一个列表项 
*               struct xLIST_ITEM * configLIST_VOLATILE  pxPrevious;  上一个列表项  
*           };
*           typedef struct xMINI_LIST_ITEM MiniListItem_t; 
*
*  成员解析：
*          listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE ：用于检测列表项数据完整性的宏定义。
*
*          xItemValue           ：列表项的值，这个值多用于按升序对列表中的列表项进行排序。(迷你一般设为最大值)
*
*          pxNext 和 pxPrevious ：分别用于指向列表中列表项的下一个列表项和上一个列表项。
*          
****************************************************************************************************/


/*************************************************************************
*                       列表操作主要API介绍
*
*           vListInitialise()        初始化列表
*           vListInitialiseItem()    初始化列表项
*           vListInsertEnd()         列表末尾插入列表项
*           vListInsert()            列表插入列表项
*           uxListRemove()           列表移除列表项
***************************************************************************/

/***********************************************************************************
*                    vListInitialise()介绍
*
*      此函数用于初始化列表，在定义列表之后，需要先对其进行初始化， 只有初始化后的列表，
*  才能够正常地被使用。列表初始化的过程，其实就是初始化列表中的成员变量。 
*
*  函数原型  ：void vListInitialise(List_t * const pxList);
*  函数参数  ：pxList    列表
*  函数返回值： 无
************************************************************************************/

/***********************************************************************************
*                     vListInitialiseItem()介绍
*
*      此函数用于初始化列表项，如同列表一样，在定义列表项之后，也需要先对其进
*  行初始化，只有初始化有的列表项，才能够被正常地使用。列表项初始化的过程，也
*  是初始化列表项中的成员变量。 
*
*  函数原型  ：void vListInitialiseItem(ListItem_t * const pxItem);
*  函数参数  ：pxItem    列表项
*  函数返回值： 无
************************************************************************************/

/***********************************************************************************
*                      vListInsertEnd()介绍
*
*      此函数用于将待插入列表的列表项插入到列表 pxIndex 指针指向列表项
*  的前面，是一种无序的插入方法。 
*
*  函数原型  ：void vListInsertEnd(
*                                  List_t * const     pxList,
*                                  ListItem_t * const pxNewListItem);
*  函数参数  ：
*             pxList          列表
*             pxNewListItem   待插入列表项
*  函数返回值： 无
************************************************************************************/

/***********************************************************************************
*                      vListInsert()介绍
*
*      此函数用于将待插入列表的列表项按照列表项值升序排序的顺序，有序
*  地插入到列表中。
*
*  函数原型  ：void vListInsert(
*                              List_t * const     pxList,
                               ListItem_t * const pxNewListItem);
*  函数参数  ：
*             pxList          列表
*             pxNewListItem   待插入列表项
*  函数返回值： 无
************************************************************************************/

/***********************************************************************************
*                      uxListRemove()介绍
*
*      此函数用于将列表项从列表项所在列表中移除。
*
*  函数原型  ：UBaseType_t uxListRemove(ListItem_t * const pxItemToRemove);
*  函数参数  ：pxItemToRemove  待移除的列表项
*  函数返回值：整数   待移除列表项移除后，所在列表剩余列表项的数量
************************************************************************************/

/**********************************************************************************
*                             列表和数组的区别
*
*  列表相当于链表，列表项相当于节点，FreeRTOS 中的列表是一个双向环形链表 
*
*  列表的特点：
*             列表项间的地址非连续的，是人为的连接到一起的。列表项的数目
*             是由后期添加的个数决定的，随时可以改变。
*  数组的特点：
*             数组成员地址是连续的，数组在最初确定了成员数量后期无法改变。
*
*  在OS中任务的数量是不确定的，并且任务状态是会发生改变的，所以非常适用列
*  表(链表)这种数据结构。
**********************************************************************************/

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
* 测试列表和测试列表项定义
*/
List_t          TestList;           /* 定义测试列表 */
ListItem_t      ListItem1;          /* 定义测试列表项1 */
ListItem_t      ListItem2;          /* 定义测试列表项2 */
ListItem_t      ListItem3;          /* 定义测试列表项3 */


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
    /* 第一步初始化列表和列表项 */
    vListInitialise(&TestList);         /* 初始化列表 */
    vListInitialiseItem(&ListItem1);    /* 初始化列表项1 */
    vListInitialiseItem(&ListItem2);    /* 初始化列表项2 */
    vListInitialiseItem(&ListItem3);    /* 初始化列表项3 */
    ListItem1.xItemValue = 40;
    ListItem2.xItemValue = 60;
    ListItem3.xItemValue = 50;
    
    printf("/**************第二步：打印列表和列表项的地址**************/\r\n");
    printf("项目\t\t\t地址\r\n");
    printf("TestList\t\t0x%p\t\r\n", &TestList);
    printf("TestList->pxIndex\t0x%p\t\r\n", TestList.pxIndex);
    printf("TestList->xListEnd\t0x%p\t\r\n", (&TestList.xListEnd));
    printf("ListItem1\t\t0x%p\t\r\n", &ListItem1);
    printf("ListItem2\t\t0x%p\t\r\n", &ListItem2);
    printf("ListItem3\t\t0x%p\t\r\n", &ListItem3);
    printf("/**************************结束***************************/\r\n");
    
    printf("\r\n/*****************第三步：列表项1插入列表******************/\r\n");
    vListInsert((List_t*    )&TestList,         /* 列表 */
                (ListItem_t*)&ListItem1);       /* 列表项 */
    printf("项目\t\t\t\t地址\r\n");
    printf("TestList->xListEnd->pxNext\t0x%p\r\n", (TestList.xListEnd.pxNext));
    printf("ListItem1->pxNext\t\t0x%p\r\n", (ListItem1.pxNext));
    printf("TestList->xListEnd->pxPrevious\t0x%p\r\n", (TestList.xListEnd.pxPrevious));
    printf("ListItem1->pxPrevious\t\t0x%p\r\n", (ListItem1.pxPrevious));
    printf("/**************************结束***************************/\r\n");
    
    printf("\r\n/*****************第四步：列表项2插入列表******************/\r\n");
    vListInsert((List_t*    )&TestList,         /* 列表 */
                (ListItem_t*)&ListItem2);       /* 列表项 */
    printf("项目\t\t\t\t地址\r\n");
    printf("TestList->xListEnd->pxNext\t0x%p\r\n", (TestList.xListEnd.pxNext));
    printf("ListItem1->pxNext\t\t0x%p\r\n", (ListItem1.pxNext));
    printf("ListItem2->pxNext\t\t0x%p\r\n", (ListItem2.pxNext));
    printf("TestList->xListEnd->pxPrevious\t0x%p\r\n", (TestList.xListEnd.pxPrevious));
    printf("ListItem1->pxPrevious\t\t0x%p\r\n", (ListItem1.pxPrevious));
    printf("ListItem2->pxPrevious\t\t0x%p\r\n", (ListItem2.pxPrevious));
    printf("/**************************结束***************************/\r\n");
    
    printf("\r\n/*****************第五步：列表项3插入列表******************/\r\n");
    vListInsert((List_t*    )&TestList,         /* 列表 */
                (ListItem_t*)&ListItem3);       /* 列表项 */
    printf("项目\t\t\t\t地址\r\n");
    printf("TestList->xListEnd->pxNext\t0x%p\r\n", (TestList.xListEnd.pxNext));
    printf("ListItem1->pxNext\t\t0x%p\r\n", (ListItem1.pxNext));
    printf("ListItem2->pxNext\t\t0x%p\r\n", (ListItem2.pxNext));
    printf("ListItem3->pxNext\t\t0x%p\r\n", (ListItem3.pxNext));
    printf("TestList->xListEnd->pxPrevious\t0x%p\r\n", (TestList.xListEnd.pxPrevious));
    printf("ListItem1->pxPrevious\t\t0x%p\r\n", (ListItem1.pxPrevious));
    printf("ListItem2->pxPrevious\t\t0x%p\r\n", (ListItem2.pxPrevious));
    printf("ListItem3->pxPrevious\t\t0x%p\r\n", (ListItem3.pxPrevious));
    printf("/**************************结束***************************/\r\n");
    
    printf("\r\n/*******************第六步：移除列表项2********************/\r\n");
    uxListRemove((ListItem_t*   )&ListItem2);   /* 移除列表项 */
    printf("项目\t\t\t\t地址\r\n");
    printf("TestList->xListEnd->pxNext\t0x%p\r\n", (TestList.xListEnd.pxNext));
    printf("ListItem1->pxNext\t\t0x%p\r\n", (ListItem1.pxNext));
    printf("ListItem3->pxNext\t\t0x%p\r\n", (ListItem3.pxNext));
    printf("TestList->xListEnd->pxPrevious\t0x%p\r\n", (TestList.xListEnd.pxPrevious));
    printf("ListItem1->pxPrevious\t\t0x%p\r\n", (ListItem1.pxPrevious));
    printf("ListItem3->pxPrevious\t\t0x%p\r\n", (ListItem3.pxPrevious));
    printf("/**************************结束***************************/\r\n");
    
    printf("\r\n/****************第七步：列表末尾添加列表项2****************/\r\n");
    TestList.pxIndex = &ListItem1;
    vListInsertEnd((List_t*     )&TestList,     /* 列表 */
                   (ListItem_t* )&ListItem2);   /* 列表项 */
    printf("项目\t\t\t\t地址\r\n");
    printf("TestList->pxIndex\t\t0x%p\r\n", TestList.pxIndex);
    printf("TestList->xListEnd->pxNext\t0x%p\r\n", (TestList.xListEnd.pxNext));
    printf("ListItem1->pxNext\t\t0x%p\r\n", (ListItem1.pxNext));
    printf("ListItem2->pxNext\t\t0x%p\r\n", (ListItem2.pxNext));
    printf("ListItem3->pxNext\t\t0x%p\r\n", (ListItem3.pxNext));
    printf("TestList->xListEnd->pxPrevious\t0x%p\r\n", (TestList.xListEnd.pxPrevious));
    printf("ListItem1->pxPrevious\t\t0x%p\r\n", (ListItem1.pxPrevious));
    printf("ListItem2->pxPrevious\t\t0x%p\r\n", (ListItem2.pxPrevious));
    printf("ListItem3->pxPrevious\t\t0x%p\r\n", (ListItem3.pxPrevious));
    printf("/************************实验结束***************************/\r\n");

    while (1)
    {
        vTaskDelay(900);   /* 延时500个tick */
    }
}


