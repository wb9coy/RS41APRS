#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "telemetry.h"
#include "ax25.h"
#include "aprs_position.h"
#include "aprsModem.h"
#include "gps.h"
#include "bell.h"
#include "led.h"
#include "config.h"

void aprsModemSendByte(uint8_t byte)
{
	//int test;
	static int onesCount    = 0;
	uint8_t bitToSend;

	for (int i = 0; i < 8; i++)
    {
		if(byte == 0x7E)
		{
			onesCount = 0;
		}
    	if(onesCount == 5)
    	{
    		onesCount     = 0;
			bellModulateNRZI(0);
    	}

    	bitToSend = (( byte & (1 << (i) ) ) ? 1 : 0 );

		//test = (byte >> i) & 1;
		if( bitToSend )
		{
			onesCount++;
			bellModulateNRZI(1);
		}
		else
		{
			bellModulateNRZI(0);
			onesCount = 0;
		}
    }
}

void aprsModemSend()
{

#ifdef CAL_US_DELAY
	//bellDelayusTest();
#endif

	HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_RESET);

    char aprs_comment[256];
    int prefixLen = 35;

    uint8_t aprs_packet[256];
	size_t aprs_length;

	uint8_t payload[256];
	size_t payload_length;

	uint8_t ax25_payload[256];
	size_t ax25_payload_length;

	gps_data current_gps_data;

	current_gps_data = getGPSData();

    telemetry_data telemetry;
    memset(&telemetry, 0, sizeof(telemetry_data));
    telemetry.battery_voltage_millivolts = 3247;
    telemetry.internal_temperature_celsius_100 = -7 * 100;
    telemetry.temperature_celsius_100 = 24 * 100;
    telemetry.humidity_percentage_100 = 68 * 100;
    telemetry.pressure_mbar_100 = 1023 * 100;
    strlcpy(telemetry.locator, "KP21FA35jk45", sizeof(telemetry.locator));

    //$GPGGA,193839.00,3152.73494,N,11610.59406,W,2,10,0.90,24431.9,M,-34.3,M,,0000*6D
    //$GPRMC,193839.00,A,3152.73494,N,11610.59406,W,22.220,278.92,080424,,,D*4D
    telemetry.gps.time_of_week_millis = current_gps_data.time_of_week_millis;
    telemetry.gps.hours = current_gps_data.hours;
    telemetry.gps.minutes = current_gps_data.minutes;
    telemetry.gps.seconds = current_gps_data.seconds;
    telemetry.gps.week = current_gps_data.week;
    telemetry.gps.year = current_gps_data.year;
    telemetry.gps.month = current_gps_data.month;
    telemetry.gps.day = current_gps_data.day;
    telemetry.gps.leap_seconds = current_gps_data.leap_seconds;
    telemetry.gps.latitude_degrees_1000000 = current_gps_data.latitude_degrees_1000000;
    telemetry.gps.longitude_degrees_1000000 = current_gps_data.longitude_degrees_1000000;
    telemetry.gps.longitude_degrees_1000000 = current_gps_data.longitude_degrees_1000000;
    telemetry.gps.altitude_mm = current_gps_data.altitude_mm;

    memset(aprs_comment, '\0', 256);
    snprintf(aprs_comment, sizeof(aprs_comment),
            APRS_COMMENT,
            telemetry.locator,
            telemetry.temperature_celsius_100 / 100,
            telemetry.humidity_percentage_100 / 100,
            telemetry.pressure_mbar_100 / 100,
            telemetry.gps.time_of_week_millis,
            telemetry.gps.hours, telemetry.gps.minutes, telemetry.gps.seconds);
    memset(aprs_packet, '\0', 256);
    aprs_length = aprs_generate_position(aprs_packet, sizeof(aprs_packet), &telemetry, APRS_SYMBOL_TABLE,
    		                             APRS_SYMBOL, false, aprs_comment);

    memset(ax25_payload, '\0', 256);
    ax25_payload_length = ax25_encode_packet_aprs(APRS_CALLSIGN, APRS_SSID, APRS_DESTINATION, APRS_DESTINATION_SSID,
			                                 APRS_RELAYS,(char *) aprs_packet, aprs_length, ax25_payload);

    //memset(ax25_payload, 0xFF, 256);
    memset(payload, '\0', 256);
    for(int i=0;i<prefixLen;i++)
    {
    	payload[i] = 0x7E;
    }

    int j = prefixLen;
    for(int i=0;i<ax25_payload_length;i++)
    {
    	payload[j] = ax25_payload[i];
    	j++;
    }
    bellReset();
    payload_length = prefixLen + ax25_payload_length;

	for(int i = 0;i < payload_length;i++ )
	{
		aprsModemSendByte(payload[i]);
	}

	HAL_Delay(50);
	HAL_GPIO_WritePin(GPIOB, RED_LED_Pin, GPIO_PIN_SET);
}
