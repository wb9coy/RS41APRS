/*
 * cw.c
 *
 *  Created on: Jan 27, 2022
 *      Author: eswiech
 */
#include <string.h>
#include "stm32f1xx_hal.h"
#include "packetDefs.h"
#include "radio.h"

HAL_StatusTypeDef HAL_Status;
const int unit = 40;
int unitDelay  = unit;
int charDelay  = unit *4;
int wordDelay  = unit *7;
int didDelay   = unit;
int dahDelay   = unit *3;


void dit()
{
	__HAL_AFIO_REMAP_TIM15_ENABLE();
	HAL_Delay(didDelay);
	__HAL_AFIO_REMAP_TIM15_DISABLE();
	HAL_Delay(unitDelay);

}

void dah()
{
	__HAL_AFIO_REMAP_TIM15_ENABLE();
	HAL_Delay(dahDelay);
	__HAL_AFIO_REMAP_TIM15_DISABLE();
	HAL_Delay(unitDelay);
}

void sendCharacter(char c)
{
	switch(c)
	{
		case 'A':
			dit();
			dah();
			break;
		case 'B':
			dah();
			dit();
			dit();
			dit();
			break;
		case 'C':
			dah();
			dit();
			dah();
			dit();
			break;
		case 'D':
			dah();
			dit();
			dit();
			break;
		case 'E':
			dit();
			break;
		case 'F':
			dit();
			dit();
			dah();
			dit();
		case 'G':
			dah();
			dah();
			dit();
			break;
		case 'H':
			dit();
			dit();
			dit();
			dit();
			break;
		case 'I':
			dit();
			dit();
			break;
		case 'J':
			dit();
			dah();
			dah();
			dah();
			break;
		case 'K':
			dah();
			dit();
			dah();
			break;
		case 'L':
			dit();
			dah();
			dit();
			dit();
			break;
		case 'M':
			dah();
			dah();
			break;
		case 'N':
			dah();
			dit();
			break;
		case 'O':
			dah();
			dah();
			dah();
			break;
		case 'P':
			dit();
			dah();
			dah();
			dit();
			break;
		case 'Q':
			dah();
			dah();
			dit();
			dah();
			break;
		case 'R':
			dit();
			dah();
			dit();
			break;
		case 'S':
			dit();
			dit();
			dit();
			break;
		case 'T':
			dah();
			break;
		case 'U':
			dit();
			dit();
			dah();
			break;
		case 'V':
			dit();
			dit();
			dit();
			dah();
			break;
		case 'W':
			dit();
			dah();
			dah();
			break;
		case 'X':
			dah();
			dit();
			dit();
			dah();
			break;
		case 'Y':
			dah();
			dit();
			dah();
			dah();
			break;
		case 'Z':
			dah();
			dah();
			dit();
			dit();
			break;
		case '1':
			dit();
			dah();
			dah();
			dah();
			dah();
			break;
		case '2':
			dit();
			dit();
			dah();
			dah();
			dah();
			break;
		case '3':
			dit();
			dit();
			dit();
			dah();
			dah();

			break;
		case '4':
			dit();
			dit();
			dit();
			dit();
			dah();
			break;
		case '5':
			dit();
			dit();
			dit();
			dit();
			dit();
			break;
		case '6':
			dah();
			dit();
			dit();
			dit();
			dit();
			break;
		case '7':
			dah();
			dah();
			dit();
			dit();
			dit();
			break;
		case '8':
			dah();
			dah();
			dah();
			dit();
			dit();
			break;
		case '9':
			dah();
			dah();
			dah();
			dah();
			dit();
			break;
		case '0':
			dah();
			dah();
			dah();
			dah();
			dah();
			break;
		case ' ':
			HAL_Delay(wordDelay);
			break;
	}

	HAL_Delay(charDelay);
}

void sendCallCW(char * call, int len)
{
	int i;;
	__HAL_AFIO_REMAP_TIM15_DISABLE();
    HAL_Delay(1000);
	for(i=0;i< len;i++)
		sendCharacter(call[i]);
}
