#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define UART_RX_PIN 9

typedef struct {
    uint8_t hour;          ///< GMT hours
    uint8_t minute;        ///< GMT minutes
    uint8_t seconds;       ///< GMT seconds
    uint16_t milliseconds; ///< GMT milliseconds
    uint8_t year;          ///< GMT year
    uint8_t month;         ///< GMT month
    uint8_t day;           ///< GMT day

    float latitude;        ///< Floating point latitude value in degrees/minutes
    float longitude;       ///< Floating point longitude value in degrees/minutes

    int32_t latitude_fixed;  ///< Fixed point latitude in decimal degrees. Divide by 10000000.0 to get a double.
    int32_t longitude_fixed; ///< Fixed point longitude in decimal degrees. Divide by 10000000.0 to get a double.

    float latitudeDegrees;  ///< Latitude in decimal degrees
    float longitudeDegrees; ///< Longitude in decimal degrees
    float geoidheight;      ///< Difference between geoid height and WGS84 height
    float altitude;         ///< Altitude in meters above MSL
    float speed;            ///< Current speed over ground in knots
    float angle;            ///< Course in degrees from true north

    char lat;               ///< N/S
    char lon;               ///< E/W
    char mag;               ///< Magnetic variation direction
    bool fix;               ///< GPS fix
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
    .mag = 'X',
    .fix = false
};

// Function to print GPS data
void print_gps_data(const GPS_Data *data) {
    // Print each field of the GPS_Data structure
    printf("Time: %02d:%02d:%02d.%03d\n", data->hour, data->minute, data->seconds, data->milliseconds);
    printf("Latitude: %.6f %c\n", data->latitudeDegrees, data->lat);
    printf("Longitude: %.6f %c\n", data->longitudeDegrees, data->lon);
    // Add similar lines for other fields

    // Example: Print fix status
    if (data->fix) {
        printf("Fix: Yes\n");
    } else {
        printf("Fix: No\n");
    }
}

int main() {   
    
    // Configure for all NMEA sentences to be printed, page 12 https://cdn-shop.adafruit.com/datasheets/PMTK_A11.pdf
    const char configurations[] = "$PMTK314,1,1,1,1,1,5,0,0,0,0,0,0,0,0,0,0,0,0,0*2C\r\n";

    stdio_init_all();

    // Set up UART with a baud rate
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);

    // Send the configuration to the GPS
    uart_puts(UART_ID, configurations);

    
    while (1) {
   
        // Read data from the Neo M8N
        char data = uart_getc(UART_ID);

        // Print the received data
        printf("%c", data);

        // Check for the end of a line (newline character)
        if (data == '\n') {
            // Add a newline character to visually separate NMEA sentences
            printf("\n");
        }
    }

    return 0;
}
