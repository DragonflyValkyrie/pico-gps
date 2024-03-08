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
    float speed;            ///< Current speed over ground in knots
    float angle;            ///< Course in degrees from true north

    char lat;               // N / S
    char lon;               // E / W
    bool fix;               // GPS fix
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
    .speed = 0.0,
    .angle = 0.0,
    .lat = 'X',
    .lon = 'X',
    .fix = false,
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

void parse_GPGGA(const char *sentence, GPS_Data *data) {
    // Assuming a valid GPGGA sentence format: $GPGGA,hhmmss.sss,llll.lll,a,yyyyy.yyy,b,xx,yy.y,x.x,x.x,M,x.x,M,x.x,xxxx*hh\r\n

    // Parse the sentence using sscanf, skipping fields related to HDOP, geoidal separation, and age of DGPS data
    int result = sscanf(sentence, "$GPGGA,%2hhu%2hhu%2hhu.%3hu,%f,%c,%f,%c,%1hhu,%d,,,,*%2hhx",
                        &data->hour, &data->minute, &data->seconds, &data->milliseconds,
                        &data->latitude, &data->lat, &data->longitude, &data->lon,
                        &data->fix, &data->satellites_in_view);

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

void parse_GPGLL(const char *sentence, GPS_Data *data) {


}

void parse_GPVTG(const char *sentence, GPS_Data *data) {


}

void parse_GPRMC(const char *sentence, GPS_Data *data) {


}

void parse_GPGSA(const char *sentence, GPS_Data *data) {


}

void parse_GPSV(const char *sentence, GPS_Data *data) {



}

// Function to print GPS data
void print_gps_data(const GPS_Data *data) {
    // Print each field of the GPS_Data structure
    printf("Time: %02d:%02d:%02d.%03d\n", data->hour, data->minute, data->seconds, data->milliseconds);

    // Print Latitude in Degrees/Minutes format
    printf("Latitude (Degrees/Minutes): %.4f %c\n", data->latitude, data->lat);

    // Print Longitude in Degrees/Minutes format
    printf("Longitude (Degrees/Minutes): %.4f %c\n", data->longitude, data->lon);

    // Print Latitude in Decimal Degrees format
    printf("Latitude (Decimal Degrees): %.6f %c\n", data->latitudeDegrees, data->lat);

    // Print Longitude in Decimal Degrees format
    printf("Longitude (Decimal Degrees): %.6f %c\n", data->longitudeDegrees, data->lon);

    printf("Altitude: %.2f meters\n", data->altitude);
    printf("Speed: %.2f knots\n", data->speed);
    printf("Course: %.2f degrees\n", data->angle);

    // Print number of satellites
    printf("Satellites in view: %d\n", data->satellites_in_view);

    // Example: Print fix status
    if (data->fix) {
        printf("Fix: Yes\n");
    } else {
        printf("Fix: No\n");
    }
}

int main() {   
    const char configurations[] = "$PMTK314,1,1,1,1,1,5,0,0,0,0,0,0,0,0,0,0,0,0,0*2C\r\n";

    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);

    uart_puts(UART_ID, configurations);

    char sentence_buffer[256];  // Adjust the size as needed
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
                GPS_Data data;
                parse_GPGGA(sentence_buffer, &data);
                print_gps_data(&data);
            }

            // Reset the buffer for the next sentence
            sentence_index = 0;
        }
    }

    return 0;
}
