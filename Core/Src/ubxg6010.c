/*
 * ubxg6010.c
 *
 *  Created on: Nov 21, 2024
 *      Author: gene
 */

#include <string.h>
#include "ubxg6010.h"

struct NavSol     NavSolMsg;
struct NavStatus  NavStatusMsg;
struct NavPosLLH  NavPosLLHMsg;
struct NavVelNed  NavVelNedMsg;
struct NavUTCTime NavUTCTimeMsg;
struct NavGPSTime NavGPSTimeMsg;

void calculateCheckSum(const uint8_t* in, size_t length, uint8_t* out)
{

uint8_t a = 0;
uint8_t b = 0;

	for (uint8_t i = 0; i < length; i++)
	{
		a = a + in[i];
		b = b + a;
	}

	out[0] = (a & 0xFF);
	out[1] = (b & 0xFF);

}


struct NavPosLLH ubxg6010GetNavPosLLHMsg()
{
	return NavPosLLHMsg;
}

struct NavStatus ubxg6010NavStatusMsg()
{
	return NavStatusMsg;
}

struct NavSol ubxg6010GetNavSolMsg()
{
	return NavSolMsg;
}

struct NavUTCTime ubxg6010GetNavUTCTimeMsg()
{
	return NavUTCTimeMsg;
}

struct NavGPSTime ubxg6010GetNavGPSTimeMsg()
{
	return NavGPSTimeMsg;
}

struct NavVelNed ubxg6010GetNavVelNedMsg()
{
	return NavVelNedMsg;
}

HAL_StatusTypeDef checkForAck(UART_HandleTypeDef *uart, uint8_t class_id, uint8_t msg_id)
{
	HAL_StatusTypeDef HAL_Status;

	uint8_t rspBuf[8];
	uint8_t expectedRsp[8];
	uint8_t rxByte;
	uint16_t received;
	uint16_t retryCount     = 0;
	uint16_t retryLimit     = GPS_BAUD_RATE/8;

	memset(&rspBuf, 0, sizeof(rspBuf));
	while( retryCount <  retryLimit)
	{
		HAL_Status = HAL_UARTEx_ReceiveToIdle(uart, &rxByte, sizeof(rxByte), &received, 3000);
		if(rxByte == UBX_SYNC_BYTE_1)
		{
			HAL_Status = HAL_UARTEx_ReceiveToIdle(uart, &rxByte, sizeof(rxByte), &received, 3000);
			if(rxByte == UBX_SYNC_BYTE_2)
			{
				for(int i=0; i<sizeof(expectedRsp);i++)
				{
					HAL_Status = HAL_UARTEx_ReceiveToIdle(uart, &rspBuf[i], sizeof(rxByte), &received, 3000);
				}

				expectedRsp[0] = MSG_CLASS_ACK;
				expectedRsp[1] = MSG_ID_ACK_ACK;
				expectedRsp[2] = 0x2;
				expectedRsp[3] = 0x00;
				expectedRsp[4] = class_id;
				expectedRsp[5] = msg_id;
				calculateCheckSum(rspBuf,sizeof(rspBuf)- SIZEOF_CHECKSUM ,&expectedRsp[6]);
				for(int i =0;i<sizeof(expectedRsp);i++)
				{
					if(expectedRsp[i] != rspBuf[i])
					{
						HAL_Status = HAL_ERROR;
					}
				}
				if(HAL_Status == HAL_OK)
				{
					break;
				}
			}
		}
		retryCount++;
		HAL_Status     = HAL_ERROR;
	}

	return HAL_Status;
}

HAL_StatusTypeDef ubxg6010Reset(UART_HandleTypeDef *uart, uint16_t nav_bbr_mask, uint8_t reset_mode)
{
	HAL_StatusTypeDef HAL_Status;
	struct CfgRst     CfgRstMsg;

    CfgRstMsg.header.sync1           = UBX_SYNC_BYTE_1;
    CfgRstMsg.header.sync2           = UBX_SYNC_BYTE_2;
    CfgRstMsg.header.message_class   = MSG_CLASS_CFG;
    CfgRstMsg.header.message_id      = MSG_ID_CFG_RST;
    CfgRstMsg.header.payload_length  = sizeof(CfgRstMsg)-sizeof(struct UbloxHeader)- sizeof(CfgRstMsg.checksum);

    CfgRstMsg.nav_bbr_mask    = nav_bbr_mask;
	// Startup Modes
	// Hotstart 0x000
	// Warmstart 0x0001
	// Coldstart 0xFFFF
    CfgRstMsg.reset_mode      = reset_mode;
	// Reset Modes:
	// Hardware Reset 0x00
	// Controlled Software Reset 0x01
	// Controlled Software Reset - Only GPS 0x02
	// Hardware Reset After Shutdown 0x04
	// Controlled GPS Stop 0x08
	// Controlled GPS Start 0x09
    CfgRstMsg.reserved        = 0x00;
    const uint8_t* msg_ptr = (unsigned char*) &CfgRstMsg;
    calculateCheckSum(msg_ptr+SIZEOF_SYNC,sizeof(CfgRstMsg)- SIZEOF_SYNC - sizeof(CfgRstMsg.checksum)  ,CfgRstMsg.checksum);
	HAL_Status = HAL_UART_Transmit(uart,msg_ptr,sizeof(CfgRstMsg),1000);
	if(HAL_Status == HAL_OK)
	{
		HAL_Status =  checkForAck(uart, CfgRstMsg.header.message_class, CfgRstMsg.header.message_id);
	}

	HAL_Delay(1000);

	return HAL_Status;
}

HAL_StatusTypeDef ubxg6010ConfigureMessageRate(UART_HandleTypeDef *uart, uint8_t class_id, uint8_t msg_id, uint8_t rate)
{
	HAL_StatusTypeDef HAL_Status;
	struct CfgMsgRate CfgMsgRateMsg;

	CfgMsgRateMsg.header.sync1           = UBX_SYNC_BYTE_1;
	CfgMsgRateMsg.header.sync2           = UBX_SYNC_BYTE_2;
	CfgMsgRateMsg.header.message_class   = MSG_CLASS_CFG;
	CfgMsgRateMsg.header.message_id      = MSG_ID_CFG_MSG;
	CfgMsgRateMsg.header.payload_length  = sizeof(CfgMsgRateMsg)-sizeof(struct UbloxHeader)- sizeof(CfgMsgRateMsg.checksum);

	CfgMsgRateMsg.message_class = class_id;
	CfgMsgRateMsg.message_id    = msg_id;
	CfgMsgRateMsg.rate          = rate;
    const uint8_t* msg_ptr = (unsigned char*) &CfgMsgRateMsg;
    calculateCheckSum(msg_ptr+SIZEOF_SYNC,sizeof(CfgMsgRateMsg)- SIZEOF_SYNC - sizeof(CfgMsgRateMsg.checksum),CfgMsgRateMsg.checksum);
	HAL_Status = HAL_UART_Transmit(uart,msg_ptr,sizeof(CfgMsgRateMsg),1000);
	if(HAL_Status == HAL_OK)
	{
		HAL_Status =  checkForAck(uart, CfgMsgRateMsg.header.message_class, CfgMsgRateMsg.header.message_id);
	}

	return HAL_Status;
}

HAL_StatusTypeDef ubxg6010ConfigureNavEngine(UART_HandleTypeDef *uart)
{
	HAL_StatusTypeDef HAL_Status;
	struct CfgNav5 CfgNav5Msg;

	CfgNav5Msg.header.sync1           = UBX_SYNC_BYTE_1;
	CfgNav5Msg.header.sync2           = UBX_SYNC_BYTE_2;
	CfgNav5Msg.header.message_class   = MSG_CLASS_CFG;
	CfgNav5Msg.header.message_id      = MSG_ID_CFG_NAV5;
	CfgNav5Msg.header.payload_length  = sizeof(CfgNav5Msg)-sizeof(struct UbloxHeader)- sizeof(CfgNav5Msg.checksum);

	CfgNav5Msg.mask           = 0b0000001111111111; // Configure all settings
	CfgNav5Msg.dynamic_model  = 6; // Dynamic model: Airborne with <1g Acceleration
	CfgNav5Msg.fix_mode       = 2; // Fix mode: 3D only
	CfgNav5Msg.fixed_altitude = 0; // Fixed altitude (mean sea level) for 2D fix mode.
	CfgNav5Msg.fixed_altitude_variance = 10000; // Fixed altitude variance for 2D mode.
	CfgNav5Msg.min_elevation  = 5;   // Minimum Elevation for a GNSS satellite to be used in NAV (degrees)
	CfgNav5Msg.dead_reckoning_limit = 0;   // Maximum time to perform dead reckoning (linear extrapolation) in case of GPS signal loss (seconds)
	CfgNav5Msg.pdop           = 100; // Position DOP Mask to use (was 25)
	CfgNav5Msg.tdop           = 100; // Time DOP Mask to use (was 25)
	CfgNav5Msg.pos_accuracy_mask = 100; // Position Accuracy Mask (m)
	CfgNav5Msg.time_accuracy_mask = 300; // Time Accuracy Mask (m)
	CfgNav5Msg.static_hold_threshold = 0; // Static hold threshold (cm/s)
	CfgNav5Msg.dgps_timeout          = 2; // DGPS timeout, firmware 7 and newer only
	CfgNav5Msg.reserved2 = 0;
	CfgNav5Msg.reserved3 = 0;
	CfgNav5Msg.reserved4 = 0;

    const uint8_t* msg_ptr = (unsigned char*) &CfgNav5Msg;
    calculateCheckSum(msg_ptr+SIZEOF_SYNC,sizeof(CfgNav5Msg)- SIZEOF_SYNC - sizeof(CfgNav5Msg.checksum),CfgNav5Msg.checksum);
	HAL_Status = HAL_UART_Transmit(uart,msg_ptr,sizeof(CfgNav5Msg),1000);
	if(HAL_Status == HAL_OK)
	{
		HAL_Status =  checkForAck(uart, CfgNav5Msg.header.message_class, CfgNav5Msg.header.message_id);
	}

	return HAL_Status;
}

HAL_StatusTypeDef ubxg6010ProcessGPSData(UART_HandleTypeDef *uart)
{
	HAL_StatusTypeDef HAL_Status;
	uint8_t gpsRxBuf[GPS_BAUD_RATE/8];
	uint16_t received;

	uint8_t msgClass;
	uint8_t msgId;
	uint16_t payloadLen;

	int startByte = 0;

	memset(&gpsRxBuf, 0, sizeof(gpsRxBuf));
	HAL_Status = HAL_UARTEx_ReceiveToIdle(uart, gpsRxBuf, sizeof(gpsRxBuf), &received, 3000);
	for(int i=0;i<received;i++)
	{
		if(gpsRxBuf[i] == UBX_SYNC_BYTE_1)
		{
			startByte = i;
			i++;
			if(gpsRxBuf[i] == UBX_SYNC_BYTE_2)
			{
				i++;
				msgClass = gpsRxBuf[i];
				i++;
				msgId    = gpsRxBuf[i];
				i++;
				memmove(&payloadLen,&gpsRxBuf[i],2);
				i+=2;
				if(msgClass == MSG_CLASS_NAV && msgId == MSG_ID_NAV_SOL)
				{
					memmove(&NavSolMsg,&gpsRxBuf[startByte],payloadLen+sizeof(struct UbloxHeader)+SIZEOF_CHECKSUM);
				}
				if(msgClass == MSG_CLASS_NAV && msgId == MSG_ID_NAV_STATUS)
				{
					memmove(&NavStatusMsg,&gpsRxBuf[startByte],payloadLen+sizeof(struct UbloxHeader)+SIZEOF_CHECKSUM);
				}
				if(msgClass == MSG_CLASS_NAV && msgId == MSG_ID_NAV_POSLLH)
				{
					memmove(&NavPosLLHMsg,&gpsRxBuf[startByte],payloadLen+sizeof(struct UbloxHeader)+SIZEOF_CHECKSUM);
				}
				if(msgClass == MSG_CLASS_NAV && msgId == MSG_ID_NAV_VELNED)
				{
					memmove(&NavVelNedMsg,&gpsRxBuf[startByte],payloadLen+sizeof(struct UbloxHeader)+SIZEOF_CHECKSUM);
				}
				if(msgClass == MSG_CLASS_NAV && msgId == MSG_ID_NAV_TIMEUTC)
				{
					memmove(&NavUTCTimeMsg,&gpsRxBuf[startByte],payloadLen+sizeof(struct UbloxHeader)+SIZEOF_CHECKSUM);
				}
			}
		}
	}


	return HAL_Status;
}
