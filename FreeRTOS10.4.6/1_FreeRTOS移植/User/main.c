#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "key.h"
#include "led.h"
//FreeRTOS相关文件
#include "FreeRTOS.h"
#include "task.h"


int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	USART1_Init(115200);
	delay_init(72);
	KEY_Init();
	LED_Init();
	LED0_OFF;
	LED1_OFF;

	while(1);
}


