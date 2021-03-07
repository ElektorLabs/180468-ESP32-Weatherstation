#include "WindSensor.h"

WindSensor::WindSensor(int _speedPin, int _dirPin) {
  speedPin = _speedPin;
  dirPin = _dirPin;
}

void WindSensor::initWindSensor() {
  pinMode(speedPin, INPUT);
  pinMode(dirPin, ANALOG);
}

void WindSensor::setWindSpeedTimeout(unsigned long nWindSpeedTimeout) {
  windSpeedTimeout = nWindSpeedTimeout;
}

void WindSensor::updateWindSensor() {
  if (lastWindSpeedUpdate + windSpeedTimeout < millis()) {
    windSpeed = 0;
  }

  //add to average value
  addWindSpeedAvg();
}

void WindSensor::determineWindDir() {
  int dir = analogRead(dirPin);
  for (int i = 0; i < 8; i++) {
    if ((dir > (winDirVal[i] - D_MARGIN)) && (dir < (winDirVal[i] + D_MARGIN))) {
      windDir = i;
      lastDir = i;
      
      //add to average value
      addWindDirAvg();
      return;
    }
  }
  windDir = -1;

  addWindDirAvg();
}

int WindSensor::getWindDir() {
  return (windDir != -1) ? windDir : lastDir; //return the last known value if the current value is -1 (undefined)
}

void WindSensor::addWindDirAvg() {
  //add the new measurement to the previous measurements and calculate the average value
  float windDirAngle = (float)getWindDir() * PI / 4; //calculate wind direction to angle
  windDirAvgCnt++;
  float x = cos(windDirAvg * PI / 180) * (windDirAvgCnt - 1) / windDirAvgCnt + cos((float)windDirAngle) / windDirAvgCnt;
  float y = sin(windDirAvg * PI / 180) * (windDirAvgCnt - 1) / windDirAvgCnt + sin((float)windDirAngle) / windDirAvgCnt;
  windDirAvg = atan2(y, x) * 180 / PI;
}

float WindSensor::getWindDirAvg(bool clearVars) {
  float temp = windDirAvg + (windDirAvg < 0 ? 360 : 0);
  if (clearVars) {
    windDirAvgCnt = 0;
    windDirAvg = 0;
  }
  return temp;
}

String WindSensor::getWindDirString() {
  switch (getWindDir()) {
    case D_N:
      return "North";
    case D_NE:
      return "North east";
    case D_E:
      return "East";
    case D_SE:
      return "South east";
    case D_S:
      return "South";
    case D_SW:
      return "South west";
    case D_W:
      return "West";
    case D_NW:
      return "North west";
    default:
      return "Undefined";
  }
}

void WindSensor::calcWindSpeed() {
  //Serial.println("updating windspeed");
  unsigned long currMillis = millis();
  float diff = (currMillis - lastWindSpeedUpdate) * 2; //sensor pulses twice per rotation
  lastWindSpeedUpdate = currMillis;
  if (diff > 10) { //diff > 0.01 s -> 100 hz -> 34 m/s -> 122.4 km/h
    //Serial.println("diff: " + String(diff) + "ms");
    diff /= 1000;
    //Serial.println("diff: " + String(diff) + "s");
    float hz = 1.0/diff;
    //Serial.println("hz: " + String(hz));
    windSpeed = hz * 0.66; //2.4km/h for 1 rot/s -> 2.4/3.6=0.66m/s
    //Serial.println("windSpeed: " + String(windSpeed));
  }
}

float WindSensor::getWindSpeedAvg(bool clearVars) {
  float temp = windSpeedAvg;
  if (clearVars) {
    windSpeedAvgCnt = 0;
    windSpeedAvg = 0;
  }
  return temp;
}

float WindSensor::getWindSpeed() {
  return windSpeed;
}

void WindSensor::addWindSpeedAvg() {
  windSpeedAvgCnt++;
  windSpeedAvg = windSpeedAvg * (windSpeedAvgCnt - 1) / windSpeedAvgCnt + windSpeed / windSpeedAvgCnt;
}

int WindSensor::getBeaufort() {
  float kmh = windSpeed * 3.6;
  for (int i = 0; i < 12; i++) {
    if (kmh < beaufort[i])
      return i;
  }
  return 12;
}

String WindSensor::getBeaufortDesc() {
  switch(getBeaufort()) {
    case 0:
      return "Calm";
    case 1:
    case 2:
      return "Light";
    case 3:
    case 4:
      return "Moderate";
    case 5:
      return "Fresh";
    case 6:
    case 7:
      return "Strong";
    case 8:
    case 9:
      return "Gale";
    case 10:
    case 11:
      return "Storm";
    case 12:
      return "Hurricane";
    default:
      return "";
  }
}

