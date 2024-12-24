#ifndef INC_BELL_H_
#define INC_BELL_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"

#define BELL_SPACE_TONE 0
#define BELL_MARK_TONE  1

void bellReset();
void bellModulateNRZI(uint16_t bit);
void bellInitialize(TIM_HandleTypeDef *htim1,TIM_HandleTypeDef *htim15);
uint16_t bellModemPwmGetPeriod(uint16_t toneType);

#endif /* INC_BELL_H_ */
