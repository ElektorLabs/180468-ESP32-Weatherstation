#  180468 - ESP32 Weatherstation

This is the ( experimental ) Software for the ESP32 base Elektor weatherstation ( 180468 )


## Getting Started

Download the source and open it with the arduino ide ( >= 1.8.x ) and select ESP32 Pico-D4 as target.
The station shall be detected via usb as serial port. Set the ide to use the port and compile the sketch.



## Prerequisites

#### Librarys requiered:
 
##### From Library Manager
 * Adafruit BME280 Library 1.0.7 by Adafruit
 * Adafruit VEML6070 Library 
 * Adafruit TSL2561 Library
 * Adafruit Unified Sensor 1.0.2 by Adafruit
 * ArduinoJson 6.x.x by Benoit Blanchon
 * PubSubClient by Nick O'Leary 
 * CRC32 by Christopher Baker
  
###### Requiered patches to the libraries
 * For the PubSubClient , go to your library folder and search for the PubSubClient. Inside its folder search in 'src/' folder for PubSubClient.h and look for the following line:
       
        #define MQTT_MAX_PACKET_SIZE 128
   
   and change it to

        #define MQTT_MAX_PACKET_SIZE 256
   
   Now safe the file. This will allow MQTT Messages with up to 256 byte lenght instead of only 128. 
   

##### Install manual:
 * SDS011 by Elektor ( found in the folder )

After uploaidng the sketch you also need to upload the content from the data folder. Use the EPS32 Sketch Data upload tool for arduino to do so. After that reset the station and if not configured for you wifi it will show up as new network.

