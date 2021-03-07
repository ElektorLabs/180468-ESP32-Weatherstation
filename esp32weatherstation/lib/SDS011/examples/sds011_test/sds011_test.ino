//code tested on ESP32 Pico Kit

#include <SDS011.h>

SDS011 sds(Serial1);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  delay(500);
  Serial.flush();
  if (sds.setMode(SDS_SET_QUERY))
    Serial.println("mode set");
  else
    Serial.println("failed");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("getting firmware version");
  bool mode = false;
  float p25 = 0, p10 = 0;
  bool sleepMode = false;
  int per = 0;
  byte ver[3] = {0};
  
  sds.getMode(&mode);
  delay(100);
  sds.getData(&p25, &p10);
  delay(100);
  sds.getSleepMode(&sleepMode);
  delay(100);
  sds.getWorkingPeriod(&per);
  delay(100);
  sds.getFirmwareVersion(ver);

  Serial.println("mode: " + String(mode));
  Serial.println("data: p2.5: " + String(p25) + ", p10: " + String(p10));
  Serial.println("sleepmode: " + String(sleepMode));
  Serial.println("period: " + String(per));
  Serial.println("version: " + String(ver[2]) + "/" + String(ver[1]) + "/" + String(ver[0]));
  delay(5000);
}
