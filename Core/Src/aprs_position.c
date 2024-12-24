#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aprs.h"
#include "aprs_position.h"

size_t aprs_generate_position(uint8_t *payload, size_t length, telemetry_data *data,
        char symbol_table, char symbol, bool include_timestamp, char *comment)
{
    char timestamp[12];

    int16_t la_degrees, lo_degrees;
    uint8_t la_minutes, la_h_minutes, lo_minutes, lo_h_minutes;

    convert_degrees_to_dmh(data->gps.latitude_degrees_1000000 / 10, &la_degrees, &la_minutes, &la_h_minutes);
    convert_degrees_to_dmh(data->gps.longitude_degrees_1000000 / 10, &lo_degrees, &lo_minutes, &lo_h_minutes);

    int16_t heading_degrees = (int16_t) ((float) data->gps.heading_degrees_100000 / 100000.0f);
    int16_t ground_speed_knots = (int16_t) (((float) data->gps.ground_speed_cm_per_second / 100.0f) * 3.6f / 1.852f);
    int32_t altitude_feet = (data->gps.altitude_mm / 1000) * 3280 / 1000;

    if (include_timestamp) {
        aprs_generate_timestamp(timestamp, sizeof(timestamp), data);
    } else {
        strncpy(timestamp, "!", sizeof(timestamp));
    }

    aprs_packet_counter++;
                           //:!3256.92NI11715.63W#RasPi iGate Digi
    //strcpy((char *)payload,"!0000.00S/00000.00WO000/000/A=000000/P3S0T35V2479C00 RS41ng radiosonde firmware test");
    //strcpy((char *)payload,"!3256.92N/11715.62WOTest");
    //return strlen((char *)payload);
    return snprintf((char *) payload,
            length,
            ("%s%02d%02d.%02u%c%c%03d%02u.%02u%c%c%03d/%03d/A=%06d/P%dS%dT%02dV%04dC%02d%s"),
            timestamp,
            abs(la_degrees), la_minutes, la_h_minutes,
            la_degrees > 0 ? 'N' : 'S',
            symbol_table,
            abs(lo_degrees), lo_minutes, lo_h_minutes,
            lo_degrees > 0 ? 'E' : 'W',
            symbol,
            heading_degrees,
            ground_speed_knots,
            (int) altitude_feet,
            aprs_packet_counter,
            data->gps.satellites_visible,
            (int) data->internal_temperature_celsius_100 / 100,
            data->battery_voltage_millivolts,
            (int16_t) ((float) data->gps.climb_cm_per_second / 100.0f),
            comment
    );
}
