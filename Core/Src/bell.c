#include <stdint.h>
#include "bell.h"
#include "led.h"
#include "config.h"

TIM_HandleTypeDef *bellPWMTimer;
TIM_HandleTypeDef *delayTimer;
uint16_t bellTones[2];
uint16_t previousTone = BELL_SPACE_TONE;


uint16_t bellModemPwmGetPeriod(uint16_t toneType)
{
	return bellTones[toneType];
}

uint16_t bellCalculatePWMPeriod(uint32_t frequency_hz_100)
{
    return (uint16_t) (((100.0f * 1000000.0f) / (frequency_hz_100 * 2.0f))) - 1;
}

void bellReset()
{
	previousTone = BELL_SPACE_TONE;
}

void bellInitialize(TIM_HandleTypeDef *htim1,TIM_HandleTypeDef *htim15)
{
	delayTimer   = htim1;
	bellPWMTimer = htim15;

	//Bell 202
	bellTones[BELL_MARK_TONE]  = bellCalculatePWMPeriod(2200 * 100);
	bellTones[BELL_SPACE_TONE] = bellCalculatePWMPeriod(1200 * 100);
	previousTone = BELL_SPACE_TONE;
}

void bellDelayus (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(delayTimer,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(delayTimer) < us);  // wait for the counter to reach the us input in the parameter
}


void bellModulateNRZI(uint16_t bit)
{
	uint16_t toneToSend;
	uint32_t calVal     = 0;
	uint32_t delayVal   = 0;

	if(bit == 0)
	{
		if(previousTone == BELL_SPACE_TONE)
		{
			toneToSend = BELL_MARK_TONE;
		}
		else
		{
			toneToSend = BELL_SPACE_TONE;
		}
	}
	else
	{
		toneToSend = previousTone;
	}


	previousTone = toneToSend;

	__HAL_TIM_SetAutoreload(bellPWMTimer,bellTones[toneToSend]);
	calVal = __HAL_TIM_GET_COUNTER(delayTimer);
	delayVal = symbol_delay_bell_202_1200bps_us - calVal;
	bellDelayus( delayVal);
	__HAL_TIM_SET_COUNTER(delayTimer,0);

#ifdef CAL_US_DELAY
	HAL_GPIO_TogglePin(GPIOB, GREEN_LED_Pin);
#endif
}
