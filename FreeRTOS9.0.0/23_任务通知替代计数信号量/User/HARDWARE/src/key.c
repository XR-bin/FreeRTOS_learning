#include "stm32f10x.h"
#include "key.h"

static void delay_ms(u8 ms)
{
	u32 i;
	for(i=0; i< ms*100; i++);
}

/*****************************************************
*函数功能  ：对KEY对应的GPIO口进行初始化设置
*函数名    ：KEY_Init
*函数参数  ：void
*函数返回值：void
*描述      ：
*            KEY0     PE4
*            KEY1     PE3
*            KEY_UP   PA0
*            KEY0、KEY1 低电平按下，高电平抬起
*            KEY_UP     高电平按下，低电平抬起
*                                 -------------输出模式
********************************************************/
void KEY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;         //GPIOx配置结构体
	
	/*GPIOx设置*/
  //GPIOx时钟使能  GPIOA、GPIOE
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
  //GPIOx配置
  //PE
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4;	    //PE3/4端口配置
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 		        //上拉输入
  GPIO_Init(GPIOE, &GPIO_InitStructure);					          //根据设定参数初始化GPIE PE3/4  
  //PA
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	                //PA0端口配置
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD ; 		        //下拉输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);					          //根据设定参数初始化GPIOA PA0
}

/*****************************************************
*函数功能  ：按键扫描函数
*函数名    ：KEY_Scan
*函数参数  ：void
*函数返回值：u8
*描述      ：
             KEY0     PE4
             KEY1     PE3
             KEY_UP   PA0
             KEY0、KEY1 低电平按下，高电平抬起
             KEY_UP     高电平按下，低电平抬起
                                  -------------输出模式
********************************************************/
u8 KEY_Scan(void)
{
  u8 temp = 0;
  static u8 key_flay = 1;
  if(key_flay && (KEY_UP||!(KEY1)||!(KEY0)))
  {
    delay_ms(10);
    if(KEY_UP||!(KEY1)||!(KEY0))
    {
      key_flay = 0;
      if(!(KEY0))        temp = 1;
      else if(!(KEY1))   temp = 2;
      else if(KEY_UP)    temp = 3;
    }
  }
  if(!(KEY_UP) && (KEY1) && (GPIOE->IDR & (1<<4)))
  {
    key_flay = 1;
  }
  return temp;
}



/*****************************************************
*函数功能  ：按键KEY0扫描函数
*函数名    ：KEY0_Scan
*函数参数  ：void
*函数返回值：u8
*描述      ：
             KEY0     PE4
             KEY0 低电平按下，高电平抬起
                                  -------------输出模式
********************************************************/
u8 KEY0_Scan(void)
{
  // 检测是否有按键按下
	if(KEY0 == 0)  
	{	
		delay_ms(3);
		if(KEY0 == 0)
		{
			//等待按键释放
			while(KEY0 == 0);   
			return 	1;	
		} 
	}

	return 0;
}


/*****************************************************
*函数功能  ：按键KEY1扫描函数
*函数名    ：KEY1_Scan
*函数参数  ：void
*函数返回值：u8
*描述      ：
             KEY1     PE3
             KEY1 低电平按下，高电平抬起
                                  -------------输出模式
********************************************************/
u8 KEY1_Scan(void)
{
  // 检测是否有按键按下
	if(KEY1 == 0)  
	{	
		delay_ms(3);
		if(KEY1 == 0)
		{
			//等待按键释放
			while(KEY1 == 0);   
			return 	2;
		}	 
	}

	return 0;
}





