# Hydration Station Electrical and Embedded software

This GitHub repository holds the KiCad files for the Hydration Station circuit board and the embedded software running on the Nano and the ESP32.



# Electrical Overview

A majority of the electrical design is in the circuit board. This board was designed in KiCad. To open the project, open KiCad and use KiCad to open the `Full Hydration Station.kicad\_pro` in the `Hydration Station Circuit` folder. This will allow you open up the schematic and PCB layout for the Hydration Station circuit board.



The design focuses on using two microcontroller breakout boards: the Arduino Nano and ESP32 board. The Arduino Nano handles control of the solenoids through the MOSFETs as well as the LCD display through I2C, and the ESP32 handles communication with the website. The two boards communicate with each other through I2C. This link is primarily used so the Arduino can tell the ESP32 when bottles get cleaned, filled, etc. and then gets forwarded to the website through HTTP requests.



There are a number of sensors, which all have corresponding 2.54mm header breakouts connecting to either the ESP32 or the Arduino. These include a weight sensor for soap level detection, a LCD for interfacing with the user, buttons for interacting with the Hydration Station, a water flow sensor, and a hall effect sensor for detecting when the door is closed.



# Software Overview

Currently, the code for the Arduino Nano is the only one that has been implemented. This code implements a simple state machine to handle cleaning. Work still needs to be done on the software side to get the sensors working, getting the ESP32 connected with the website, and getting the two microcontrollers communicating with each other.

