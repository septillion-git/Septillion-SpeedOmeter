# Septillion-SpeedOmeter
Speedometer for model railroads. Can use different scales, detectors, lengths, displays etc

## Settings
Settings can be made at the start of the .ino. Settings include:

* scale
* detection track length
* used detection pins
* Active high or active low for detection
* used display 
* timings

## Displays
SpeedOmeter supportsmultiple display types. New types can be added easy and in the future new types will be added.

At this moment the following types are supported:

1. SSD1306 128x64 OLED display (I2C, default address 0x3C, [Adafruit SSD1306 library](https://github.com/adafruit/Adafruit_SSD1306))
2. Adafruit 7-segment display (I2C, default address is 0x70, [Adafruit-LED-Backpack-Library](https://github.com/adafruit/Adafruit-LED-Backpack-Library))

You need the corresponding library to make it work.

## Install
1. Download Septillion-SpeedOmeter (hit download .ZIP) and extract the folder in your Arduino Sketchbook (default C:\Users\[username]\My Documents\Arduino\)
2. Download the library of the display you want to use and import in Arduino
3. Change the settings to meet your needs
4. Compile and upload
