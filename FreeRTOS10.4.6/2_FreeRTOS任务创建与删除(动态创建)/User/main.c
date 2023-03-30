#include "stm32f10x.h"
#include "usart.h"
#include "FreeRTOS_demo.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    USART1_Init(115200);
    
    App_demo();

    while(1);
}


