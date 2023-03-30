#include "stm32f10x.h"
#include "usart.h"
#include "stdio.h"

/*****************************************************
*函数功能  ：对USART1对应的GPIO口进行初始化设置
*函数名    ：USART1_Init
*函数参数  ：u32 baud
*函数返回值：void
*描述      ：
*           PA9     TX     ---------输出
*           PA10    RX     ---------输入
********************************************************/
void USART1_Init(u32 baud)
{
  GPIO_InitTypeDef  GPIO_InitStruct;     //GPIOx配置结构体
  USART_InitTypeDef USART_InitStruct;    //USARTx配置结构体
  
  //时钟使能   GPIOA   USART1
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE); 
  
  /*GPIOx初始化设置*/
  //GPIOx端口配置
  //PA9   USART1_TX ----- 复用串口输出
  GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_9;				     //PA9 端口配置
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP; 	  	 //复用推挽输
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		   //IO口速度为50MHz
  GPIO_Init(GPIOA, &GPIO_InitStruct);                  //根据设定参数初始化PA9
  //PA10  USART1_RX ----- 复用串口输入
  GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_10;				     //PA10 端口配置
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;  //浮空输入
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		   //IO口速度为50MHz
  GPIO_Init(GPIOA, &GPIO_InitStruct);                  //根据设定参数初始化PA10
  
  /*USART 初始化设置*/
  //USART1配置
  USART_InitStruct.USART_BaudRate      = baud;                 //波特率设置(USART->BRR寄存器)
  USART_InitStruct.USART_WordLength    = USART_WordLength_8b;  //字长为8位数据格式:一个起始位， 8个数据位， n个停止位；(USART->CR1寄存器的第12位)
  USART_InitStruct.USART_StopBits      = USART_StopBits_1;     //一个停止位(USART->CR2寄存器的第12、13位)
  USART_InitStruct.USART_Parity        = USART_Parity_No;      //无奇偶校验(USART->CR1寄存器的第10位)
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制(USART->CR3寄存器的第9位)
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式(USART->CR1寄存器的第2、3位)
  USART_Init(USART1,&USART_InitStruct);                        //初始化USART1
  
  //使能串口1
  USART_Cmd(USART1, ENABLE);
}

/*****************************************************
*函数功能  ：串口1发送一个字节的数据（u8）
*函数名    ：USART1_Send_Byte
*函数参数  ：u8 data
*函数返回值：void
*描述      ：
*           PA9     TX     ---------输出
*           PA10    RX     ---------输入
********************************************************/
void USART1_Send_Byte(u8 data)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET);//等待发送结束(USART1->SR的第6位)
	USART_SendData(USART1, data);                           //发送一个字节的数据(USART1->DR)
}

/*****************************************************
*函数功能  ：串口1接收一个字节的数据（u8）
*函数名    ：USART1_Receive_Byte
*函数参数  ：void
*函数返回值：u8
*描述      ：
********************************************************/
u8 USART1_Receive_Byte(void)
{
  u8 data;
	
  while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)!=SET){};//等待发送结束(USART1->SR的第5位)
  data = USART_ReceiveData(USART1);                         //读取一个字节的数据(USART1->DR)
  
  return data;
}

/******************************************************************
*函数功能  ：串口1发送一个字符串
*函数名    ：USART1_Send_Str
*函数参数  ：u8 *str
*函数返回值：void
*描述      ：
            PA9   TX    输出
            PA10  RX    输入
*******************************************************************/
void USART1_Send_Str(u8 *str)
{
  while(*str != '\0')
  {
    USART1_Send_Byte(*str);
    str++;
  }
}

/******************************************************************
*函数功能  ：串口1接收一个字符串
*函数名    ：USART1_Receive_Str
*函数参数  ：u8 *str
*函数返回值：void 
*描述      ：
            PA9   TX    输出
            PA10  RX    输入
*******************************************************************/
void USART1_Receive_Str(u8 *str)
{
  u8 data;
  while(1)
  {
    data = USART1_Receive_Byte();
    if(data == '#')   //以 # 为结束标志
    {
      break;
    }
    *str = data;
    str++;
  }
  *str = '\0'; 
}

























///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}








