/*
 * config.h
 *
 *  Created on: Oct 25, 2021
 *      Author: eswiech
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#define VERSION_INFO ((char *)"Version 1.0")

#define CALL_SIGN ((char *)"WB9COY")

#define TRANSMIT_FREQUENCY  432.5000f //Mhz

// TX Power
#define TX_POWER  0 // PWR 0...7 0- MIN ... 7 - MAX
// Power Levels measured at 434.650 MHz, using a Rigol DSA815, and a 10 kHz RBW
// Power measured by connecting a short (30cm) length of RG316 directly to the
// antenna/ground pads at the bottom of the RS41 PCB.
// 0 --> -1.9dBm
// 1 --> 1.3dBm
// 2 --> 3.6dBm
// 3 --> 7.0dBm
// 4 --> 10.0dBm
// 5 --> 13.1dBm - DEFAULT
// 6 --> 15.0dBm
// 7 --> 16.3dBm
//#define CAL_US_DELAY true
#define symbol_delay_bell_202_1200bps_us 828
//#define symbol_delay_bell_202_1200bps_us 823
// Number of character pairs to include in locator
#define LOCATOR_PAIR_COUNT_FULL 6 // max. 6 (12 characters WWL)

#define APRS_CALLSIGN "KA7NSR"
#define APRS_SSID 'B'
// See APRS symbol table documentation in: http://www.aprs.org/symbols/symbolsX.txt
#define APRS_SYMBOL_TABLE '/' // '/' denotes primary and '\\' denotes alternate APRS symbol table
#define APRS_SYMBOL 'O'
#define APRS_COMMENT "ANSR Flight"
#define APRS_RELAYS "WIDE1-1,WIDE2-1" // Do not include any spaces in the APRS_RELAYS
#define APRS_DESTINATION "APZ41N"
#define APRS_DESTINATION_SSID '0'
// Generate an APRS weather report instead of a position report. This will override the APRS symbol with the weather station symbol.
#define APRS_WEATHER_REPORT_ENABLE false

#endif /* INC_CONFIG_H_ */
