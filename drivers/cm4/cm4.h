  
#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f37x.h"

// CAN bus defines for cortex-M4 STM32F373

#define CANx                       CAN1
#define CAN_CLK                    RCC_APB1Periph_CAN1
#define CAN_RX_PIN                 GPIO_Pin_8
#define CAN_TX_PIN                 GPIO_Pin_9
#define CAN_GPIO_PORT              GPIOB
#define CAN_GPIO_CLK               RCC_AHBPeriph_GPIOB
#define CAN_AF_PORT                GPIO_AF_9
#define CAN_RX_SOURCE              GPIO_PinSource8
#define CAN_TX_SOURCE              GPIO_PinSource9


#endif 

