#ifndef INC_APRSMODEM_H_
#define INC_APRSMODEM_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"

void aprsModemInitialize(TIM_HandleTypeDef *htim1,TIM_HandleTypeDef *htim15);
void aprsModemTone(uint16_t pwmValue);
uint16_t aprsModemPwmGetPeriod(uint16_t toneType);
void aprsModemSend();
void aprsModemDelayusTest();

#endif /* INC_APRSMODEM_H_ */
