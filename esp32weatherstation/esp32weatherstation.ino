



/******************************************************************************************
 * 
 * Elektor ESP32 based Weaterstation 
 * HW: EPS32-PICO-D4 on PCB 180468
 * 
 * Librarys requiered:
 * 
 * From Library Manager
 * Adafruit BME280 Library 1.0.7 by Adafruit
 * Adafruit Unified Sensor 1.0.2 by Adafruit
 * ArduinoJson 6.x.x by Benoit Blanchon
 * PubSubClient by Nick O'Leary 
 * CRC32 by Christopher Baker
 * 
 *********************************************************************************************/


#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "sslCertificate.h"
#include <WebServer.h>
#include <SPIFFS.h>
#include <Preferences.h>
/* I2C Sensors */
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML6070.h>
#include <Adafruit_TSL2561_U.h>

#include <Wire.h>
#include <SDS011.h>
#include "Honnywell.h"
#include "WindSensor.h"
#include "RainSensor.h"
#include "datastore.h"
#include "mqtt_task.h"
#include "NTP_Client.h"
#include "timecore.h"

#define solarRelay 18
#define measBatt 34

#define APPin 22
#define APLed 19
#define STALed 23
#define windDirPin 32
#define windSpeedPin 33
#define rainPin 27

#define battLow 11
#define battFull 13.5
#define battInterval 2000

#define bmeAddress 0x76
#define bmeInterval 5000 //ms = 5 seconds

#define sdsInterval 60000 //ms = 1 minute

#define lastConnectedTimeout 600000 //ms = 10 minutes: 10 * 60 * 1000

#define hourMs 3600000 //ms (60 * 60 * 1000 ms)


//thingspeak api and fields numbers
//https://thingspeak.com/channels/535447/private_show
bool thingspeakEnabled;
String thingspeakApi;
int tsfWindSpeed = 0;
int tsfWindDir = 0;
int tsfRainAmount = 0;
int tsfTemperature = 0;
int tsfHumidity = 0;
int tsfAirpressure = 0;
int tsfPM25 = 0;
int tsfPM10 = 0;

//senseBox IDs
bool senseBoxEnabled;
String senseBoxStationId;
String senseBoxTempId;
String senseBoxHumId;
String senseBoxPressId;
String senseBoxWindSId;
String senseBoxWindDId;
String senseBoxRainId;
String senseBoxPM25Id;
String senseBoxPM10Id;

unsigned long uploadInterval = hourMs;
bool uploaded = false;


bool prevWindPinVal = false;
bool prevRainPinVal = false;

//current sensor values
int windDir = 0; //degrees
float windSpeed = 0; //m/s
int beaufort = 0;
String beaufortDesc = "";
float windSpeedAvg = 0;
float windDirAvg = 0;
float rainAmountAvg = 0;

float temperature = 0; //*C
float humidity = 0; //%
float pressure = 0; //hPa
bool bmeRead = 0;

int16_t uv_level = 0;

float lux_level = 0;
bool has_TSL2561 = true;

float PM10 = 0; //particle size: 10 um or less
float PM25 = 0; //particle size: 2.5 um or less

float batteryVoltage = 0; //v
float batteryCharging = false;

//serial variables
String serialIn;
bool serialRdy = false;
bool volatile hasBME280 = false;
unsigned long lastBMETime = 0;
unsigned long lastSDSTime = 0;
unsigned long lastUploadTime = 0;
unsigned long lastAPConnection = 0;
unsigned long lastBattMeasurement = 0;





WindSensor ws(windSpeedPin, windDirPin);
RainSensor rs(rainPin);
Adafruit_BME280 bme;
Adafruit_VEML6070 uv = Adafruit_VEML6070();
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);


#define USE_SDS011
//#define USE_HONNYWELLHPM

#ifdef USE_SDS011
/* If SDS011 is connected */
SDS011 sds(Serial1);
#endif

#ifdef USE_HONNYWELLHPM
/* If Honnywhell sensor is connected */
/* Warning Code is untested and not supported */
HonnywellHPM hpm(Serial1);
#endif

//webserver, client and secure client pointers
WebServer* server = NULL;
WiFiClient* client = NULL;
WiFiClientSecure* clientS = NULL;



Preferences pref;

Timecore timec;
NTP_Client NTPC;

Ticker TimeKeeper;

void _1SecondTick( void );

void setup() {
  //first we enable the dynamic frequency scaling to reduce power consumption
  /*
  esp_pm_config_esp32_t dynamicsettings;
  dynamicsettings.maxfreq_mhz = ESP32_DEFAULT_CPU_FREQ_80;
  dynamicsettings.min_freq_mhz = 10000000; 
  dynamicsettings.light_sleep_enable = false;
  if(ESP_OK  != esp_pm_configure(&dynamicsettings){
    
  }
  */
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  
  SPIFFS.begin();
  Serial.println("SPIFFS started");

  datastoresetup();
  
  pinMode(APPin, INPUT_PULLUP);
  pinMode(APLed, OUTPUT);
  pinMode(STALed, OUTPUT);
  pinMode(solarRelay, OUTPUT);
  pinMode(measBatt, ANALOG);
  
  digitalWrite(APLed, LOW);
  digitalWrite(STALed, LOW);
  digitalWrite(solarRelay, LOW);
 
  loadUploadSettings();
  loadNetworkCredentials();
  initWiFi();
  
  ws.initWindSensor();
  rs.initRainSensor();
  
  /* We read the Config from flash */
  Serial.println(F("Read Timecore Config"));
  timecoreconf_t cfg = read_timecoreconf();
  timec.SetConfig(cfg);
  /* Now we start with the config for the Timekeeping and sync */
  TimeKeeper.attach_ms(1000, _1SecondTick);
  

  Wire.begin(25, 26, 100000); //sda, scl, freq=100kHz
  if(false == bme.begin(bmeAddress)){
        hasBME280 = false;
  } else {
    hasBME280 = true;
    //recommended settings for weather monitoring
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF);
  }
    if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.println("No TSL2561 detected, searched on ADDR:0x39, ( addr pin floating )");
    has_TSL2561 = false;
  } else {
     tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
     /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
     //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
     // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
     tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
     has_TSL2561 = true;
  }

  uv.begin(VEML6070_1_T);  // pass in the integration time constant
  

  #ifdef USE_SDS011 
    sds.setMode(SDS_SET_QUERY);
    Serial.println("Using SDS011");
  #endif

  #ifdef USE_HONNYWHELLHPM
    Serial.println("Using Honnywell HPM");
    hpm.begin();
  #endif
  MQTTTaskStart();


 
  
}

void loop() {
  //serial debugging
  checkSerial();
  
  NetworkTask();

  //if the current wifi mode isn't STA and the timout time has passed -> restart
  if ((WiFi.getMode() != WIFI_STA) && ((lastAPConnection + lastConnectedTimeout) < millis())) {
    if (digitalRead(APPin)) {
      Serial.println("Last connection was more than 10 minutes ago. Restart.");
      ESP.restart();
    }
    else {//button still pressed
      lastAPConnection = millis(); //wait some time before checking again
    }
  }

  //reinit if the wifi connection is lost
  if ((WiFi.getMode() == WIFI_STA) && (WiFi.status() == WL_DISCONNECTED)) {
    digitalWrite(STALed, LOW);
    Serial.println("Lost connection. Trying to reconnect.");
    initWiFi();
  }

  //read sensors
  readWindSensor();
  readRainSensor();

  //read bme280 every 5 seconds, also UV and Light
  if ((lastBMETime + bmeInterval) < millis()) {
    lastBMETime = millis();
    readBME();
    readUV();
    readLux();
  }

  //read SDS011 every minute
  #ifdef USE_SDS011
  if ((lastSDSTime + sdsInterval) < millis()) {
    lastSDSTime = millis();
    if (!sds.getData(&PM25, &PM10))
      Serial.println("Failed to read SDS011");
  }
  #endif

  #ifdef HONNYWELLHPM
    hpm.ProcessData();
    if ((lastSDSTime + sdsInterval) < millis()) {
      lastSDSTime = millis();
      if (!hpm.getData(&PM25, &PM10))
        Serial.println("Failed to read HPM");
      }
   
  #endif
  
  if(uploadInterval< 60*1000 ){
        uploadInterval = 60 *1000;    
  }

  //upload data if the uploadperiod has passed and if WiFi is connected
  if (((lastUploadTime + uploadInterval) < millis()) && (WiFi.status() == WL_CONNECTED)) {
    
    lastUploadTime = millis();
    Serial.println("Upload interval time passed");
    if( (true ==thingspeakEnabled) || ( true == senseBoxEnabled) ) {

        /* For Thingspeak and SneseBox we are using idx 0  */
        windSpeedAvg = ws.getWindSpeedAvg();
        windDirAvg = ws.getWindDirAvg();
        rainAmountAvg = rs.getRainAmount(0) * hourMs / uploadInterval;
      
        if (thingspeakEnabled) {
          if (uploadToThingspeak())
            Serial.println("Uploaded successfully");
          else
            Serial.println("Uploading failed");
        }
        else
          Serial.println("Thingspeak disabled");

        if (senseBoxEnabled) {
          if (uploadToSenseBox())
            Serial.println("Uploaded successfully");
          else
            Serial.println("Uploading failed");
        }
        else
          Serial.println("SenseBox disabled");
        }
  } else {
    Serial.println("SenseBox  and Thingspeak disabled");
  }

  //handle battery (resistor divider: vBatt|--[470k]-+-[100k]--|gnd)
  if ((lastBattMeasurement + battInterval) < millis()) {
    lastBattMeasurement = millis();
    float adcVoltage = ((float)analogRead(measBatt)/4096) * 3.3 + 0.15; //0.15 offset from real value
    batteryVoltage = adcVoltage * 570 / 100 + 0.7; //analog read between 0 and 3.3v * resistor divider + 0.7v diode drop
    //Serial.println("adc voltage: " + String(adcVoltage) + ", batt voltage: " + String(batteryVoltage) + ", currently charging: " + String(batteryCharging ? "yes" : "no"));
    if (batteryVoltage > battFull)
      batteryCharging = false;
    if (batteryVoltage < battLow)
      batteryCharging = true;
    
    digitalWrite(solarRelay, batteryCharging);
  }
}

//reads the windsensor and stores the values in global variables
void readWindSensor() {
  if (digitalRead(windSpeedPin) && !prevWindPinVal) {
    ws.calcWindSpeed();
  }
  prevWindPinVal = digitalRead(windSpeedPin);

  ws.updateWindSensor();
  windSpeed = ws.getWindSpeed();
  beaufort = ws.getBeaufort();
  beaufortDesc = ws.getBeaufortDesc();

  ws.determineWindDir();
  windDir = ws.getWindDir();
}

void readRainSensor() {
  //inverted logic
  if (!digitalRead(rainPin) && prevRainPinVal) {
    Serial.println("Rainbucket tipped");
    rs.calcRainAmount();
  }
  prevRainPinVal = digitalRead(rainPin);
}

void readBME() {
  if( hasBME280 == true ){
    bme.takeForcedMeasurement();
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0;
  } else {
    humidity=0;
    pressure=0;
  }
}

void readUV( void ){
  uint16_t level = uv.readUV();
  uv_level = level; // library can return -1 in case of an error but it is casted to uint16_t ........
}

void readLux( void ){
  if(has_TSL2561==false){
   lux_level=0;
  } else {
  sensors_event_t event;
    tsl.getEvent(&event);
   
    /* Display the results (light is measured in lux) */
    if (event.light)
    {
      //Serial.print(event.light); Serial.println(" lux");
      lux_level = event.light;
      
    }
  }
}
 

/**************************************************************************************************
 *    Function      : _1SecondTick
 *    Description   : Runs all fnctions inside once a second
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void _1SecondTick( void ){
     timec.RTC_Tick();  
}
