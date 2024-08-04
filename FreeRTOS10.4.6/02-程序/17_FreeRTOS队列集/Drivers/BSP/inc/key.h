#ifndef __KEY_H
#define __KEY_H

    /****************   外部头文件声明   ****************/
    #include "sys.h"
    #include "delay.h"



    /********************   宏定义   ********************/
    #define KEY0    GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)
    #define KEY1    GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)
    #define KEY_UP  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)



    /****************    函数外部声明   *****************/
    void KEY_Init(void);
    uint8_t KEY_Scan(void);

#endif












