  
#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f0xx.h"

// CAN bus defines for STM32F072

#define CANx                       CAN
#define CAN_CLK                    RCC_APB1Periph_CAN
#define CAN_RX_PIN                 GPIO_Pin_11
#define CAN_TX_PIN                 GPIO_Pin_12
#define CAN_GPIO_PORT              GPIOA
#define CAN_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define CAN_AF_PORT                GPIO_AF_4
#define CAN_RX_SOURCE              GPIO_PinSource11
#define CAN_TX_SOURCE              GPIO_PinSource12       

#endif 

