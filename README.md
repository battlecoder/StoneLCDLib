# StoneLCDLib
This library is intended to make it easier to use the [STONE](https://www.stoneitech.com) LCD HMI Displays with Arduino-compatible boards.

For the time being it comes with an equivalent to the [RGB LED Strip Example](https://www.stoneitech.com/application/civil-electronic/ws2812b-rgb-lamp-control-display-module.html) that comes with the LCD screens.

This library currently has the following features:
* Read and write to/from LCD registers
* Read and write to/from User-defined variables.
* Receive events from the LCD.
* Read, Write and Parse the on-board RTC clock data.

## Missing Features
The following is a short list of pending work:
* Support for CRC checks
* Functions to use the graph/curve buffer
* [*] High level functions for audio/video playback
* [*] High level functions for reading touchscreen data
* [*] High level functions for Page switching

With the exception of the first two items, all of the remaining features are controlled by LCD registers, so they can be performed with the already existing functions in this library. I hope, however, to add more methods especifically designed to access those features in a friendlier way.

## Compatibility
This library doesn't use any device-specific feature, so it should be compatible with all the devices supported by the base Arduino framework. Having said that, it has only been tested with AVR-based Arduino boards.

## Usage
Once installed, add this line to the top of your Arduino sketch:
```
#include <StoneLCDLib.h>
```


### 1. Initialization
Initialize the Library by creating a StoneLCD object, passing the serial port it's connected to:
```
StoneLCD myLCD(&Serial);
```
Serial can be a Serial port implementation that complies with Arduino's [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/) interface. This means that you can also use a [SoftSerial](https://www.arduino.cc/en/Reference/softwareSerial) port, if care is taken to constantly check for incoming bytes. 

You can also specify the command packet Lo/Hi bytes when you initialize the LCD:
```
StoneLCD myLCD(&Serial, 0xA5, 0x5A);
```

### 2. Reading / Writing LCD Registers
The current functions are meant to be used to work with the LCD registers:
* writeRegister (regStartAddr, *buffer, buffLen)
* writeRegisterByte (regStartAddr, b)
* writeRegisterWord (regStartAddr, w)
* readRegister (regStartAddr, *dest_buffer, buffLen)
* readRegisterByte (regStartAddr)
* readRegisterWord (regStartAddr)

These should be sufficient to set or retrieve LCD parameters, and control features like media playback, Touchscreen, RTC clock, etc.

*StoneLCDLib.h* also contains definitions for the addresses of most registers (e.g: STONE_REG_TP_STATUS, STONE_REG_RUNTIME, STONE_REG_VOL, etc). Check the file for a list of available constants.

### 3. Reading / Writing Variables
User variables can be accessed through the following methods.


### 4. RTC
You can access the RTC through the registers directly, but this library provides an abstraction class plus read/write methods so you can handle the RTC Date/Time data easily.

Methods:
* getRTC (*StoneLCDDateTime)
* setRTC (*StoneLCDDateTime)

The *StoneLCDDateTime* provides the following functionality:
* getYear()
* getMonth()
* getDay()
* getWeek()
* getHour()
* getMinutes()
* getSeconds()
* setYear(y)
* setMonth(m)
* setDay(d)
* setWeek(w)
* setHour(h)
* setMinutes(m)
* setSeconds(s)

### 5. Receiving events from the screen

Make sure that you are constantly calling *checkForIOEvents()* somewhere in your main loop.
When you call this function you'll need to specify a buffer for the event data, and a pointer to a StoneLCDEvent object that will receive the basic event information, like this:

```
StoneLCDEvent evt;
uint16_t recvBuffer[8];
boolean recvEvent;

recvEvent = myLCD.checkForIOEvent(&evt, recvBuffer, 8);
```
