#ifndef __APRS_H
#define __APRS_H

#include <stddef.h>
#include <stdint.h>

#include "gps.h"
#include "telemetry.h"

void convert_degrees_to_dmh(long x, int16_t *degrees, uint8_t *minutes, uint8_t *h_minutes);
void aprs_generate_timestamp(char *timestamp, size_t length, telemetry_data *data);
int16_t encode_aprs_position(char *callsign, float latitude, float longitude, char *symbol,char *packet);

extern volatile uint16_t aprs_packet_counter;

#endif
