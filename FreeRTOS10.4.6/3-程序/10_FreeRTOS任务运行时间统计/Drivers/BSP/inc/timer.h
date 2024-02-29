#ifndef __TIMER_H
#define __TIMER_H

    /****************   外部头文件声明   ****************/
    #include "sys.h"
    #include "usart.h"



    /****************    变量外部声明   *****************/
    extern uint32_t FreeRTOSRunTimeTicks;        /* FreeRTOS任务运行时间统计变量 */



    /****************    函数外部声明   *****************/
    void TIM6_Init(uint16_t arr, uint16_t psc);      /* 定时器6初始化 */
    void TIM7_Init(uint16_t arr, uint16_t psc);      /* 定时器7初始化 */
    void ConfigureTimeForRunTimeStats(void);         /* 统计FreeRTOS任务运行时间的时基定时器初始化 */

#endif
