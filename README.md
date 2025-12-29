### About the Project

The project is a wireless lighting system that synchronizes with live orchestral performances, allowing lights to be conducted in real-time.

The code has two main components:

- code that converts signals from a MIDI keyboard into variables that are sent to the Arduino IDE Serial Monitor

- the transmitter (ESP32)/receiver (ESP8266) code that sends the data over Wi-Fi using the ESP-NOW protocol, then interprets them as light sequences 

### Setup/Libraries Needed:

#### For MIDI Signal Handling Code (in Java Processing IDE):

The PianoConnector1_18 and MidiMessageHandler files need to be in the same folder

Install the following through: Processing → Sketch → Import Library → Manage Libraries:

Serial

ControlP5

The MidiBus

#### Warning: 
PianoConnector1_18 uses the port COM4 in the line "myport = new Serial(this, "COM4", 115200);"
If you are using a different port you will have to replace COM4 with a different port name. Use "printArray(Serial.list());" to see which ports you have available.

#### For the transmitter/receiver code (in Arduino IDE):

##### Use the ESP32 Arduino Nano Board

In Arduino IDE, install through File → Preferences → Additional boards manager URLS: https://dl.espressif.com/dl/package_esp32_index.json

Then, Tools → Boards Manager → Install esp32 by Espressif, version 3.1.1

##### Use the ESP8266 Board

In Arduino IDE, install through File → Preferences → Additional boards manager URLS: http://arduino.esp8266.com/stable/package_esp8266com_index.json

Then, Tools → Boards Manager → Install NodeMCU 1.0 (ESP-12E Module) by Espressif, version 3.1.2

The ESP8266 Arduino Core provides:

- ESP8266WiFi.h

- espnow.h

The ESP32 Arduino Core provides:

- WiFi.h

- esp_now.h

Install FastLED.h through: Sketch → Manage Libraries → search FastLED by Daniel, version 3.5.0
