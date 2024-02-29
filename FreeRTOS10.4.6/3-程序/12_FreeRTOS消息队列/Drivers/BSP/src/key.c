#include "key.h"



/**
******************************************************************************
* @file      ：.\Drivers\BSP\src\key.c
*              .\Drivers\BSP\inc\key.h
* @author    ：XRbin
* @version   ：V1.0
* @date      ：2023-06-16
* @brief     ：按键驱动配置代码
******************************************************************************
* @attention
*   我的GitHub   ：https://github.com/XR-bin
*   我的gitee    ：https://gitee.com/xrbin
*   我的leetcode ：https://leetcode.cn/u/xrbin/
******************************************************************************
*/



/**********************************************************
* @funcName ：KEY_Init
* @brief    ：对KEY对应的GPIO口进行初始化设置
* @param    ：void
* @retval   ：void
* @details  ：
*            KEY0     PE4
*            KEY1     PE3
*            KEY_UP   PA0
*                                 -------------输出模式
* @fn       ：
************************************************************/
void KEY_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;         /* GPIOx配置结构体 */

    /* GPIOx设置 */
    /* GPIOx时钟使能  GPIOA、GPIOE */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    /* GPIOx配置 */
    /* PE */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3|GPIO_Pin_4;  /* PE3/4端口配置 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;          /* 上拉输入 */
    GPIO_Init(GPIOE, &GPIO_InitStructure);                 /* 根据设定参数初始化GPIE PE3/4 */
    /* PA */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;             /* PA0端口配置 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;          /* 下拉输入 */
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 /* 根据设定参数初始化GPIOA PA0 */
}



/**********************************************************
* @funcName ：KEY_Scan
* @brief    ：按键扫描函数
* @param    ：void
* @retval   ：uint8_t
* @details  ：
*            KEY0、KEY1 低电平按下，高电平抬起
*            KEY_UP     高电平按下，低电平抬起
* @fn       ：
************************************************************/
uint8_t KEY_Scan(void)
{
    uint8_t temp = 0;
    static uint8_t key_flay = 1;

    if(key_flay && ( KEY_UP==1 || KEY0==0 || KEY1==0 ))
    {
        delay_ms(10);
        if( KEY_UP==1 || KEY0==0 || KEY1==0 )
        {
            key_flay = 0;
            if(KEY0==0)        temp = 1;
            else if(KEY1==0)   temp = 2;
            else if(KEY_UP==1) temp = 3;
        }
    }

    if(KEY_UP==0 && KEY0==1 && KEY1==1)
    {
        key_flay = 1;
    }

    return temp;
}











