#include "list.h"

//定义根节点
struct xLIST List_Test;
//定义数据节点
struct xLIST_ITEM  List_Item1;
struct xLIST_ITEM  List_Item2;
struct xLIST_ITEM  List_Item3;
struct xLIST_ITEM  List_Item4;


int main(void)
{
	/* 链表根节点初始化 */
	vListInitialise( &List_Test );
	
	
	
	/* 初始化链表数据节点 */
	//节点1初始化
	vListInitialiseItem( &List_Item1 );
	List_Item1.xItemValue = 1;
	//节点2初始化   
	vListInitialiseItem( &List_Item2 );
	List_Item2.xItemValue = 2;
	//节点3初始化
	vListInitialiseItem( &List_Item3 );
	List_Item3.xItemValue = 3;
	//节点4初始化
	vListInitialiseItem( &List_Item3 );
	List_Item4.xItemValue = 4;
	
	
	
	/* 将节点一个个从末尾插入 */
	//vListInsertEnd( &List_Test, &List_Item3 );    
	//vListInsertEnd( &List_Test, &List_Item2 );
	//vListInsertEnd( &List_Test, &List_Item1 ); 
	//vListInsertEnd( &List_Test, &List_Item4 ); 
	
	/* 将节点按照升序排列插入到链表 */
	//vListInsert( &List_Test, &List_Item3 );    
	//vListInsert( &List_Test, &List_Item2 );
	//vListInsert( &List_Test, &List_Item1 ); 
	//vListInsert( &List_Test, &List_Item4 ); 
	
	while(1);
}

