#include <string.h>
#include "main.h"
#include "gps.h"
#include "ubxg6010.h"
#include "ublox_structures.h"

UART_HandleTypeDef *uart;

gps_data current_gps_data;
uint8_t gpsFix;

void updateGoodPacketCount()
{
	current_gps_data.ok_packets++;
}

gps_data getGPSData()
{
	return current_gps_data;
}

HAL_StatusTypeDef gpsInit(UART_HandleTypeDef *huart)
{
	HAL_StatusTypeDef HAL_Status;

	uart = huart;

	memset(&current_gps_data, 0, sizeof(current_gps_data));

	// Coldstart 0xFFFF
	//Controlled Software Reset - Only GPS 0x02
	HAL_Status = ubxg6010Reset(uart, 0xFFFF, 0x2);

	// Dynamic model needs to be set to one of the Airborne modes to support high altitudes!
	// Tweaked the PDOP limits a bit, to make it a bit more likely to report a position.
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureNavEngine(uart);
	}
	// Rate of 1 f ==or message: 0x01 0x02 Geodetic Position Solution
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0x01, 0x02, 0x01);
	}
	// Rate of 1 for message: 0x01 0x06 Navigation Solution Information
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0x01, 0x06, 0x01);
	}
	 // Configure rate of 1 for message: 0x01 0x21 UTC Time Solution
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0x01, 0x21, 0x01);
	}
	// Configure rate of 2 for message: 0x01 0x12 Velocity Solution in NED
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0x01, 0x12, 0x02);
	}
	// Configure rate of 2 for message: 0x01 0x03 Receiver Navigation Status
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0x01, 0x03, 0x02);
	}

	//Disable NMEA
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0xF0, 0x00, 0x00);  //GGA
	}
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0xF0, 0x01, 0x00);  //GLL
	}
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0xF0, 0x02, 0x00);  //GSA
	}
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0xF0, 0x03, 0x00);  //GSV
	}
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0xF0, 0x04, 0x00);  //RMC
	}
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0xF0, 0x05, 0x00);  //VTG
	}
	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureMessageRate(uart, 0xF0, 0x08, 0x00);  //ZDA
	}

	if(HAL_Status == HAL_OK)
	{
		HAL_Status = ubxg6010ConfigureNavEngine(uart);
	}

    return HAL_Status;
}

uint8_t gpsUpdatetData()
{
	int valid = 1;
	struct NavSol     NavSolMsg;
	struct NavPosLLH  NavPosLLHMsg;
	struct NavStatus  NavStatusMsg;
	struct NavUTCTime NavUTCTimeMsg;
	struct NavGPSTime NavGPSTimeMsg;
	struct NavVelNed  NavVelNedMsg;

	uint8_t calcCheckSum[2];

	//0x01 0x02
	NavPosLLHMsg = ubxg6010GetNavPosLLHMsg();
	calculateCheckSum((const uint8_t*)&NavPosLLHMsg.header.message_class, sizeof(NavPosLLHMsg)- SIZEOF_CHECKSUM-SIZEOF_SYNC, &calcCheckSum[0]);
    if( (calcCheckSum[0] != NavPosLLHMsg.checksum[0]) || (calcCheckSum[1] != NavPosLLHMsg.checksum[1]) )
    {
    	valid = 0;
    }
    else
    {
    	current_gps_data.time_of_week_millis = NavPosLLHMsg.iTOW;
    	current_gps_data.latitude_degrees_1000000 = NavPosLLHMsg.latitude_scaled;
    	current_gps_data.longitude_degrees_1000000 = NavPosLLHMsg.longitude_scaled;
    	current_gps_data.altitude_mm = NavPosLLHMsg.height_mean_sea_level;
    }

    if(valid)
    {
		//0x01 0x03
    	NavStatusMsg = ubxg6010NavStatusMsg();
		calculateCheckSum((const uint8_t*)&NavStatusMsg.header.message_class, sizeof(NavStatusMsg)- SIZEOF_CHECKSUM-SIZEOF_SYNC, &calcCheckSum[0]);
		if( (calcCheckSum[0] != NavStatusMsg.checksum[0]) || (calcCheckSum[1] != NavStatusMsg.checksum[1]) )
		{
			valid = 0;
		}
		else
		{
			current_gps_data.fix_ok = NavStatusMsg.flags & 0x01;
			current_gps_data.power_safe_mode_state = NavStatusMsg.flags2 & 0x03;
		}
    }

    if(valid)
    {
		//0x01 0x06
		NavSolMsg = ubxg6010GetNavSolMsg();
		calculateCheckSum((const uint8_t*)&NavSolMsg.header.message_class, sizeof(NavSolMsg)- SIZEOF_CHECKSUM-SIZEOF_SYNC, &calcCheckSum[0]);
		if( (calcCheckSum[0] != NavSolMsg.checksum[0]) || (calcCheckSum[1] != NavSolMsg.checksum[1]) )
		{
			valid = 0;
		}
		else
		{
			current_gps_data.time_of_week_millis = NavSolMsg.iTOW;
			current_gps_data.week = NavSolMsg.week;
			current_gps_data.fix = NavSolMsg.gpsFix;
			current_gps_data.satellites_visible = NavSolMsg.numSV;
			current_gps_data.position_dilution_of_precision = NavSolMsg.pDop;
		}
    }

    if(valid)
    {
		//0x01 0x12
    	NavVelNedMsg = ubxg6010GetNavVelNedMsg();
		calculateCheckSum((const uint8_t*)&NavVelNedMsg.header.message_class, sizeof(NavVelNedMsg)- SIZEOF_CHECKSUM-SIZEOF_SYNC, &calcCheckSum[0]);
		if( (calcCheckSum[0] != NavVelNedMsg.checksum[0]) || (calcCheckSum[1] != NavVelNedMsg.checksum[1]) )
		{
			valid = 0;
		}
		else
		{
			current_gps_data.time_of_week_millis = NavVelNedMsg.iTOW;
			current_gps_data.ground_speed_cm_per_second = NavVelNedMsg.ground_speed;
			current_gps_data.heading_degrees_100000 = NavVelNedMsg.heading_scaled;
			current_gps_data.climb_cm_per_second = NavVelNedMsg.velocity_down;
		}
    }

    if(valid)
    {
		//0x01 0x20
    	NavGPSTimeMsg = ubxg6010GetNavGPSTimeMsg();
		calculateCheckSum((const uint8_t*)&NavGPSTimeMsg.header.message_class, sizeof(NavGPSTimeMsg)- SIZEOF_CHECKSUM-SIZEOF_SYNC, &calcCheckSum[0]);
		if( (calcCheckSum[0] != NavGPSTimeMsg.checksum[0]) || (calcCheckSum[1] != NavGPSTimeMsg.checksum[1]) )
		{
			valid = 0;
		}
		else
		{
			current_gps_data.time_of_week_millis = NavGPSTimeMsg.iTOW;
			current_gps_data.week = NavGPSTimeMsg.week;

	        if (NavGPSTimeMsg.valid & 0x04)
	        {
	            // Flag set if leap seconds are valid
	            current_gps_data.leap_seconds = NavGPSTimeMsg.leapsecs;
	        }
		}
    }

    if(valid)
    {
		//0x01 0x21
    	NavUTCTimeMsg = ubxg6010GetNavUTCTimeMsg();
		calculateCheckSum((const uint8_t*)&NavUTCTimeMsg.header.message_class, sizeof(NavUTCTimeMsg)- SIZEOF_CHECKSUM-SIZEOF_SYNC, &calcCheckSum[0]);
		if( (calcCheckSum[0] != NavUTCTimeMsg.checksum[0]) || (calcCheckSum[1] != NavUTCTimeMsg.checksum[1]) )
		{
			valid = 0;
		}
		else
		{
			current_gps_data.year = NavUTCTimeMsg.year;
			current_gps_data.month = NavUTCTimeMsg.month;
			current_gps_data.day = NavUTCTimeMsg.day;
			current_gps_data.hours = NavUTCTimeMsg.hour;
			current_gps_data.minutes = NavUTCTimeMsg.min;
			current_gps_data.seconds = NavUTCTimeMsg.sec;
		}
    }

	if (GPS_HAS_FIX(current_gps_data) && (valid))
	{
		gpsFix = 1;
		setGreenLEDToggleRate(5);
	}
	else
	{
		gpsFix = 0;
		setGreenLEDToggleRate(1);
	}

	return gpsFix;
}
