#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

#define UTX_Pin GPIO_PIN_9
#define URX_Pin GPIO_PIN_10
#define WIRE_Pin GPIO_PIN_4
#define SCL_Pin GPIO_PIN_6
#define SDA_Pin GPIO_PIN_7
#define SCK_Pin GPIO_PIN_5
#define MISO_Pin GPIO_PIN_6
#define MOSI_Pin GPIO_PIN_7
#define CS_Pin GPIO_PIN_0

#define UTX_GPIO_Port GPIOA
#define URX_GPIO_Port GPIOA
#define WIRE_GPIO_Port GPIOB
#define SCL_GPIO_Port GPIOB
#define SDA_GPIO_Port GPIOB
#define CS_GPIO_Port GPIOB
#define SCK_GPIO_Port GPIOA
#define MISO_GPIO_Port GPIOA
#define MOSI_GPIO_Port GPIOA

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
