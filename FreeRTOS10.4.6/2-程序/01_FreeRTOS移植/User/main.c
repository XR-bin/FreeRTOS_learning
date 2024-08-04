#include "sys.h"
#include "delay.h"
#include "led.h"
/* FreeRTOS相关文件 */
#include "FreeRTOS.h"
#include "task.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); /* 4号分组方案 4 位抢占优先级， 0 位响应优先级 */
    SysTick_Init(72);       /* 滴答定时器初始化，带中断 */
    LED_Init();             /* LED初始化 */

    while(1)
    {
        LED0_ON;
        LED1_ON;
        delay_ms(300);
        LED0_OFF;
        LED1_OFF;
        delay_ms(300);
    }
}


