### About the Project

The project is a lighting system for live orchestral performances using light devices that are wireless and controllable in real-time using a MIDI keyboard.

The code has two main components:

-the MIDI signal handling code that converts signals from a MIDI keyboard into variables that are sent to the Arduino IDE Serial Monitor

-the transmitter (ESP32)/receiver (ESP8266) code that sends the data over Wi-Fi using the ESP-NOW protocol, then interprets them as light sequences 

### Frameworks/Libraries Needed:

#### For MIDI Signal Handling Code (in Java Processing IDE):

Install the following through: Processing → Sketch → Import Library → Add Library:

processing.serial

controlP5

themidibus

#### For the transmitter/receiver code (in Arduino IDE):

Install via Arduino IDE → Boards Manager

ESP8266 by ESP8266 Community

ESP32 by Espressif Systems

The ESP8266 Arduino Core provides:

ESP8266WiFi.h

espnow.h

The ESP32 Arduino Core provides:

WiFi.h

esp_now.h

Install FastLED.h through: Sketch → Include Library → Manage Libraries → search “FastLED”
