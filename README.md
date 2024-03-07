# Raspberry Pi Pico(RP2040) GPS NMEA Sentence Parser
 
This C code is designed to parse NMEA sentences from a GPS module, and extract useful information such as time, latitude, longitude, altitude, speed, course, and fix status. The parsed data is then printed to the console in a human-readable format. Other satallite constelations such as GLONASS, Galileo, or BeiDou would not be parsed as cose only parses GPGGA sentences. Thiis is due because Adafruit Ultimate GPS only recieves GPGGA sentences. The parser can be edited to change format for their identifiers. 

This has been tested with Adafruit Ultimate GPS breakout board and a Pico using C/C++ SDK.

The .vscode is from the Raspberry Pi Foundation Pico Examples to set up the enivorment for windows vscode.

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
