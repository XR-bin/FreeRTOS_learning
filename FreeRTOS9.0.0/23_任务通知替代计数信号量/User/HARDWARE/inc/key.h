#ifndef _KEY_H
#define _KEY_H

  #include "stm32f10x.h"
	
	#define KEY0    GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)
  #define KEY1    GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)
  #define KEY_UP  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)
  
  void KEY_Init(void);
  u8 KEY_Scan(void);
	u8 KEY0_Scan(void);
	u8 KEY1_Scan(void);

#endif












