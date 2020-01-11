#include "test_task.h"
#include "main.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"


#include "led.h"
#include "CAN_Receive.h"
#include "delay.h"
#include "USART_comms.h"
#include "stm32f4xx.h"

/*
// uart testing
int buffer_rx[100] = {0};
int count_rx = 0;
*/

void testTask(void *pvParameters)
{
    while(1) {
				Serial_sendString("here");
				Serial_sendInt(5);
			  delay_ms(100);
    }
    
}
