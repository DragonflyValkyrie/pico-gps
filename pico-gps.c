#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <string.h>

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define UART_RX_PIN 9

typedef struct {
    uint8_t hour;          // GMT hours
    uint8_t minute;        // GMT minutes
    uint8_t seconds;       // GMT seconds
    uint16_t milliseconds; // GMT milliseconds
    uint8_t year;          // GMT year
    uint8_t month;         // GMT month
    uint8_t day;           // GMT day

    float latitude;        // Floating point latitude value in degrees/minutes
    float longitude;       // Floating point longitude value in degrees/minutes

    int32_t latitude_fixed;  // Fixed point latitude in decimal degrees. Divide by 10000000.0 to get a double.
    int32_t longitude_fixed; // Fixed point longitude in decimal degrees. Divide by 10000000.0 to get a double.

    float latitudeDegrees;  // Latitude in decimal degrees
    float longitudeDegrees; // Longitude in decimal degrees
    float geoidheight;      // Difference between geoid height and WGS84 height
    float altitude;         // Altitude in meters above MSL
    float speedknot;        // Current speed over ground in knots
    float speedkm;          // Current speed over ground in knots
    float angle;            // Course in degrees from true north
    char heading_unit_true; // True (geographic) north
    char heading_unit_mag;  // Magnetic north

    char lat;               // N / S
    char lon;               // E / W
    bool fix;               // GPS fix
    char mode;              // A = Autonomous , D = DGPS, E = DR (This field is only present in NMEA version 3.0)
    int satellites_in_view; // Satellies in view
} GPS_Data;

// Initialize a GPS_Data instance with default values
GPS_Data gps_data_default = {
    .hour = 0,
    .minute = 0,
    .seconds = 0,
    .milliseconds = 0,
    .year = 0,
    .month = 0,
    .day = 0,
    .latitude = 0.0,
    .longitude = 0.0,
    .latitude_fixed = 0,
    .longitude_fixed = 0,
    .latitudeDegrees = 0.0,
    .longitudeDegrees = 0.0,
    .geoidheight = 0.0,
    .altitude = 0.0,
    .speedknot = 0.0,
    .speedkm = 0.0,
    .angle = 0.0,
    .lat = 'X',
    .lon = 'X',
    .fix = false,
    .mode = 'X',
    .heading_unit_true = 'X',
    .heading_unit_mag = 'X',
    .satellites_in_view = 0
};


bool is_valid_nmea_sentence(const char *sentence) {
    // Check if the sentence starts with '$' and ends with '\r\n'
    if (sentence[0] != '$' || sentence[strlen(sentence) - 2] != '\r' || sentence[strlen(sentence) - 1] != '\n') {
        return false;
    }

    // Calculate and verify the checksum
    int checksum = 0;
    for (int i = 1; sentence[i] != '*' && sentence[i] != '\0'; ++i) {
        checksum ^= sentence[i];
    }

    // Get the checksum value from the sentence
    int sentence_checksum;
    if (sscanf(sentence, "%*[^*]*%x", &sentence_checksum) != 1) {
        return false; // Error extracting the checksum
    }

    return checksum == sentence_checksum;
}

// Convert latitude and longitude to decimal degrees
void convert_coords_to_decimal(GPS_Data *data) {
    // Calculate decimal degrees for latitude and longitude
    data->latitudeDegrees = (int)data->latitude / 100 + (data->latitude - (int)data->latitude) / 60;
    data->longitudeDegrees = (int)data->longitude / 100 + (data->longitude - (int)data->longitude) / 60;

    // Adjust latitude and longitude based on N/S and E/W indicators
    if (data->lat == 'S') {
        data->latitudeDegrees = -data->latitudeDegrees;
    }
    if (data->lon == 'W') {
        data->longitudeDegrees = -data->longitudeDegrees;
    }
}

void parse_GPGGA(const char *sentence, GPS_Data *data) {
    // GPGGA sentence format: $GPGGA, HHMMSS.sss, ddmm.mmmm, N/S, dddmm.mmmm, E/W, 0-2, 0-12, x.x, x.x, M, x.x, M, x.x, xxxx*cs\r\n

    // Parse the sentence, skipping fields related to HDOP, geoidal separation, and age of DGPS data
    int result = sscanf(sentence, "$GPGGA,%2hhu%2hhu%2hhu.%3hu,%f,%c,%f,%c,%1hhu,%d,,,,*%2hhx",
                        &data->hour, &data->minute, &data->seconds, &data->milliseconds,
                        &data->latitude, &data->lat, &data->longitude, &data->lon,
                        &data->fix, &data->satellites_in_view);

    convert_coords_to_decimal(data);

    int fix_type;

    data->fix = (fix_type == 1 || fix_type == 2); // 1 or 2 means a valid fix
}

void parse_GPGLL(const char *sentence, GPS_Data *data) {
    // GPGLL sentence format: $GPGLL, latitude, N/S, longitude, E/W, time, A, *cs\r\n

    char fix_indicator; // Use char to represent 'A' or 'V'

    // Parse the sentence using sscanf, skipping mode field
    int result = sscanf(sentence, "$GPGLL,%f,%c,%f,%c,%2hhu%2hhu%2hhu.%3hu,%c,%c,*%2hhx",
                        &data->latitude, &data->lat, &data->longitude, &data->lon,
                        &data->hour, &data->minute, &data->seconds, &data->milliseconds,
                        &fix_indicator, &data->mode);

    // Convert 'A' to true, 'V' to false and assign to data->fix
    data->fix = (fix_indicator == 'A') ? true : false;

    convert_coords_to_decimal(data);
}

void parse_GPVTG(const char *sentence, GPS_Data *data) {
    // GPVTG sentence format: $GPVTG, course, reference, course, reference, speed over ground, N, speed over ground, K, mode*cs\r\n
    
    int result = sscanf(sentence, "$GPVTG,%f,%c,,%*c,%f,%*c,%f,%*c,%c,*%2hhx",
                        &data->angle, &data->heading_unit_true, &data->speedknot, &data->speedkm, &data->mode);

}

void parse_GPRMC(const char *sentence, GPS_Data *data) {
    // GPRMC sentence format: $GPVTG, time, status, latitude, N/S, longitude, E/W, speed over ground, course over ground, date, magnetic variation, mode*cs\r\n

    char fix_indicator; // Use char to represent 'A' or 'V'

    int result = sscanf(sentence, "$GPRMC,%2hhu%2hhu%2hhu.%3hu,%c,%f,%c,%f,%c,%f,%f,%2hhu%2hhu%2hhu,,,%c,*%2hhx",
                        &data->hour, &data->minute, &data->seconds, &data->milliseconds, &fix_indicator,
                        &data->latitude, &data->lat, &data->longitude, &data->lon, &data->speedknot,
                        &data->angle, &data->day, &data->month, &data->year, &data->mode);

    // Convert 'A' to true, 'V' to false and assign to data->fix
    data->fix = (fix_indicator == 'A') ? true : false;

    convert_coords_to_decimal(data);
}

// Function to print GPS data
void print_gps_data(const GPS_Data *data) {
    printf("Time: %02d:%02d:%02d.%03d\n", data->hour, data->minute, data->seconds, data->milliseconds);

    printf("DDMMYYY Date: %02d/%02d/20%02d\n", data->day, data->month, data->year);

    printf("Latitude (Degrees/Minutes): %.4f %c\n", data->latitude, data->lat);

    printf("Longitude (Degrees/Minutes): %.4f %c\n", data->longitude, data->lon);

    printf("Latitude (Decimal Degrees): %.6f %c\n", data->latitudeDegrees, data->lat);

    printf("Longitude (Decimal Degrees): %.6f %c\n", data->longitudeDegrees, data->lon);

    printf("Altitude: %.2f meters\n", data->altitude);

    printf("Speed: %.2f knots\n", data->speedknot);

    printf("Speed: %.2f km\n", data->speedkm);

    printf("Course: %.2f degrees\n", data->angle);

    printf("Heading: %.2f %c\n", data->angle, data->heading_unit_true);

    printf("Mode: %c\n", data->mode);

    printf("Satellites in view: %d\n", data->satellites_in_view);

    if (data->fix) {
        printf("Fix: Yes\n");
    } else {
        printf("Fix: No\n");
    }
}

int main() {
    // Refer to https://cdn-shop.adafruit.com/datasheets/PMTK_A11.pdf for configurations
    const char configurations[] = "$PMTK314,1,1,1,1,1,5,0,0,0,0,0,0,0,0,0,0,0,0,0*2C\r\n";

    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);

    uart_puts(UART_ID, configurations);

    char sentence_buffer[256];
    int sentence_index = 0;

    while (1) {
        char data = uart_getc(UART_ID);

        // Store the received character in the buffer
        sentence_buffer[sentence_index++] = data;

        // Check for the end of a line (newline character)
        if (data == '\n') {
            // Null-terminate the buffer to make it a valid string
            sentence_buffer[sentence_index] = '\0';

            printf(sentence_buffer);

            // Process the NMEA sentence
            if (is_valid_nmea_sentence(sentence_buffer)) {
                if (strncmp(sentence_buffer, "$GPGGA", 6) == 0) {
                    parse_GPGGA(sentence_buffer, &gps_data_default);
                    print_gps_data(&gps_data_default);
                }
            }
            // Reset the buffer for the next sentence
            sentence_index = 0;
        }
    }

    return 0;
}
