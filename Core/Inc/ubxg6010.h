#ifndef __UBXG6010_H
#define __UBXG6010_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "ublox_structures.h"
#define GPS_BAUD_RATE 9600

HAL_StatusTypeDef ubxg6010Reset(UART_HandleTypeDef *huart, uint16_t nav_bbr_mask, uint8_t reset_mode);
HAL_StatusTypeDef ubxg6010ConfigureMessageRate(UART_HandleTypeDef *uart, uint8_t class_id, uint8_t msg_id, uint8_t rate);
HAL_StatusTypeDef ubxg6010ProcessGPSData(UART_HandleTypeDef *uart);
HAL_StatusTypeDef ubxg6010ConfigureNavEngine(UART_HandleTypeDef *uart);
struct NavSol ubxg6010GetNavSolMsg();
struct NavPosLLH ubxg6010GetNavPosLLHMsg();
struct NavStatus ubxg6010NavStatusMsg();
struct NavUTCTime ubxg6010GetNavUTCTimeMsg();
struct NavGPSTime ubxg6010GetNavGPSTimeMsg();
struct NavVelNed  ubxg6010GetNavVelNedMsg();
void calculateCheckSum(const uint8_t* in, size_t length, uint8_t* out);


#endif
