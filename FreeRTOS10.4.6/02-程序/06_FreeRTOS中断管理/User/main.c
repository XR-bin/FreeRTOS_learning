#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "freeRTOS_demo.h"
/* FreeRTOS相关文件 */
#include "FreeRTOS.h"
#include "task.h"

/***
* 注意：该实验设置FreeRTOS系统所能管理的优先级范围：5~15。
*/

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); /* 4号分组方案 4 位抢占优先级， 0 位响应优先级 */
    SysTick_Init(72);       /* 滴答定时器初始化，带中断 */
    USART1_Init(115200);    /* 串口1初始化 115200 */
    LED_Init();             /* LED初始化 */
    TIM6_Init(9999, 7200);  /* 定时器6初始化，1s中断一次 */
    TIM7_Init(9999, 7200);  /* 定时器7初始化，1s中断一次 */

    freeRTOS_demo();        /* freeRTOS实验 */
}


