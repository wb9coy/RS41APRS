/*
 * radio.c
 *
 *  Created on: Jan 27, 2022
 *      Author: eswiech
 */
#include <string.h>
#include "Setup_Si4032.h"
#include "radio.h"
#include "led.h"
#include "config.h"
#include "packetDefs.h"
#include "main.h"

static SPI_HandleTypeDef *hspi;

HAL_StatusTypeDef setupRadio(SPI_HandleTypeDef *hspiParam)
{
	  HAL_StatusTypeDef HAL_Status;
	  hspi = hspiParam;

	  //Soft Reset
	  HAL_Status = radio_write_register(0x07, 0x80);
	  HAL_Delay(1000);

	  //Set Tx Power
	  HAL_Status = radio_write_register(0x6D, 00 | (TX_POWER & 0x0007));

	  HAL_Status = radio_set_tx_frequency(TRANSMIT_FREQUENCY);

	  HAL_Status = EnableDirectMode();
	  // 5KHz Dev
	  HAL_Status = radio_write_register(0x72, Si4032_FREQUENCY_DEVIATION);

	  HAL_GPIO_WritePin(GPIOB, GREEN_LED_Pin, GPIO_PIN_RESET);

	 return HAL_Status;
}

HAL_StatusTypeDef enableTx()
{
	HAL_StatusTypeDef HAL_Status;

	HAL_Status = radio_write_register(0x07, Si4032_OPERATING_AND_FUNCTION_CONTROL_1);

	return HAL_Status;
}

HAL_StatusTypeDef disableTx()
{
	HAL_StatusTypeDef HAL_Status;

	HAL_Status = radio_write_register(0x07, 0x00);

	return HAL_Status;

}

void remapTIM15()
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  /**TIM15 GPIO Configuration
		PB15     ------> TIM15_CH2
	   */
	  GPIO_InitStruct.Pin = GPIO_PIN_15;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  __HAL_AFIO_REMAP_TIM15_ENABLE();
}

HAL_StatusTypeDef EnableDirectMode()
{
	HAL_StatusTypeDef HAL_Status;


	//Data Clock Config.: No TX Data Clock (only for OOK and FSK)
	//Data Source:        Direct Mode using TX_Data via SDI pin
	//Modulation Type:    FSK
	HAL_Status = radio_write_register(0x71, Si4032_MODULATION_MODE_CONTROL_2);

	//nSEL high
	HAL_GPIO_WritePin(RADIO_NSS_GPIO_Port, RADIO_NSS_Pin, GPIO_PIN_SET);

    return HAL_Status;
}

HAL_StatusTypeDef radio_read_register(const uint8_t register_addr, uint8_t *pData)
{
  HAL_StatusTypeDef HAL_Status;
  uint16_t data_word;
  uint8_t addr;
  uint8_t val;

  HAL_GPIO_WritePin(RADIO_NSS_GPIO_Port, RADIO_NSS_Pin, GPIO_PIN_RESET);

  data_word = ((register_addr | 0x0) << 8);
  addr = data_word >> 8;

  HAL_Status = HAL_SPI_Transmit(hspi, (uint8_t *)&addr, 1, 1000);
  HAL_Delay(1);
  HAL_Status = HAL_SPI_Receive(hspi, &val, 1, 1000);

  memcpy(pData,&val,1);

  HAL_GPIO_WritePin(RADIO_NSS_GPIO_Port, RADIO_NSS_Pin, GPIO_PIN_SET);

  return HAL_Status;
}

HAL_StatusTypeDef radio_write_register(const uint8_t register_addr, uint8_t value)
{
  HAL_StatusTypeDef HAL_Status;

  uint8_t WR = 0x80;
  uint16_t data_word;
  uint8_t addr;
  uint8_t val;

  HAL_GPIO_WritePin(RADIO_NSS_GPIO_Port, RADIO_NSS_Pin, GPIO_PIN_RESET);

  data_word = ((register_addr | WR) << 8) | value;
  addr = data_word >> 8;
  val  = (uint8_t)data_word;


  HAL_Status = HAL_SPI_Transmit(hspi, (uint8_t *)&addr, 1, 1000);
  HAL_Delay(1);
  HAL_Status = HAL_SPI_Transmit(hspi, (uint8_t *)&val, 1, 1000);

  HAL_GPIO_WritePin(RADIO_NSS_GPIO_Port, RADIO_NSS_Pin, GPIO_PIN_SET);

  return HAL_Status;
}

HAL_StatusTypeDef radio_write_register_burst(const uint8_t register_addr, uint8_t txData[],uint8_t len)
{
  HAL_StatusTypeDef HAL_Status;

  uint8_t WR = 0x80;
  uint16_t data_word;
  uint8_t addr;
  uint8_t val;
  int i;

  HAL_GPIO_WritePin(RADIO_NSS_GPIO_Port, RADIO_NSS_Pin, GPIO_PIN_RESET);

  data_word = ((register_addr | WR) << 8) | txData[0];
  addr = data_word >> 8;
  val  = (uint8_t)data_word;


  HAL_Status = HAL_SPI_Transmit(hspi, (uint8_t *)&addr, 1, 1000);
  HAL_Delay(1);
  HAL_Status = HAL_SPI_Transmit(hspi, (uint8_t *)&val, 1, 1000);

  for(i=1;i<len;i++)
  {
	  HAL_Status = HAL_SPI_Transmit(hspi, (uint8_t *)&txData[i], 1, 1000);
  }

  HAL_GPIO_WritePin(RADIO_NSS_GPIO_Port, RADIO_NSS_Pin, GPIO_PIN_SET);

  return HAL_Status;
}

HAL_StatusTypeDef radio_set_tx_frequency(float freq_in_mhz)
{
  HAL_StatusTypeDef HAL_Status;
  float SI4032_CLOCK = 26.0;

  uint8_t hbsel = (uint8_t) ((freq_in_mhz * (30.0f / SI4032_CLOCK)) >= 480.0f ? 1 : 0);

  uint8_t fb = (uint8_t) ((((uint8_t)((freq_in_mhz * (30.0f / SI4032_CLOCK)) / 10) - 24) - (24 * hbsel)) / (1 + hbsel));
  uint8_t gen_div  =  3;  // constant - not possible to change!
  uint16_t fc = (uint16_t) (((freq_in_mhz / ((SI4032_CLOCK / gen_div) * (hbsel + 1))) - fb - 24) * 64000);
  HAL_Status = radio_write_register(0x75, (uint8_t) (0b01000000 | (fb & 0b11111) | ((hbsel & 0b1) << 5)));
  HAL_Status = radio_write_register(0x76, (uint8_t) (((uint16_t)fc >> 8) & 0xff));
  HAL_Status = radio_write_register(0x77, (uint8_t) ((uint16_t)fc & 0xff));

  HAL_Status = radio_write_register(0x73, Si4032_FREQUENCY_OFFSET_1);
  HAL_Status = radio_write_register(0x74, Si4032_FREQUENCY_OFFSET_2);
  HAL_Status = radio_write_register(0x7A, Si4032_FREQUENCY_HOPPING_STEP_SIZE);
  HAL_Status = radio_write_register(0x79, Si4032_FREQUENCY_HOPPING_CHANNEL);

  return HAL_Status;
}
