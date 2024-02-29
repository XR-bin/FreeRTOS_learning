#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "freeRTOS_demo.h"
/* FreeRTOS相关文件 */
#include "FreeRTOS.h"
#include "task.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); /* 4号分组方案 4 位抢占优先级， 0 位响应优先级 */
    SysTick_Init(72);       /* 滴答定时器初始化，带中断 */
    USART1_Init(115200);    /* 串口1初始化 115200 */
    LED_Init();             /* LED初始化 */
    KEY_Init();             /* KEY初始化 */

    freeRTOS_demo();        /* freeRTOS实验 */
}


