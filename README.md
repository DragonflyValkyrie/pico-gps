# Raspberry Pi Pico(RP2040) GPS NMEA Sentence Parser
 
This C code is designed to parse NMEA sentences from a GPS module, and extract useful information such as time, latitude, longitude, altitude, speed, course, and fix status. The parsed data is then printed to the console in a human-readable format. Other satallite constelations such as GLONASS, Galileo, or BeiDou would not be parsed as this only parses GP sentences. This is due because Adafruit Ultimate GPS only recieves GPGGA sentences. The parser can be edited to change format for other identifiers and structure.

## Tested

This has been tested with Adafruit Ultimate GPS breakout board, NEO M8N u-blox gps, NEO 6M u-blox gps and a Pico using C/C++ SDK.

## Configuration

The .vscode is from the Raspberry Pi Foundation Pico Examples to set up the enivorment for windows vscode. This includes debugging with the pico probe.

## To build

```
mkdir build
cd build
cmake ..
make
```

## Pin out

```
5v-->VIN
Tx-->Rx
Rx-->Tx
GND-->GND
```

Tx and Rx pins used were 8 and 9 respectively
