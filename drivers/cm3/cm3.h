  
#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"

// CAN bus defines for STM32F103
/*
#define CANx                       CAN1
#define CAN_CLK                    RCC_APB1Periph_CAN1
#define CAN_RX_PIN                 GPIO_Pin_8
#define CAN_TX_PIN                 GPIO_Pin_9
#define CAN_GPIO_PORT              GPIOB
#define CAN_GPIO_CLK               RCC_APB2Periph_GPIOB
*/

// CAN bus defines for STM32F105
#define CANx                       CAN1
#define CAN_CLK                    RCC_APB1Periph_CAN1
#define CAN_RX_PIN                 GPIO_Pin_11
#define CAN_TX_PIN                 GPIO_Pin_12
#define CAN_GPIO_PORT              GPIOA
#define CAN_GPIO_CLK               RCC_APB2Periph_GPIOA



#endif 

