#include "RainSensor.h"

RainSensor::RainSensor(int _pin) {
  pin = pin;
}

void RainSensor::initRainSensor() {
  pinMode(pin, INPUT);
}

void RainSensor::clearRainAmount() {
  rainAmount = 0;
}

void RainSensor::calcRainAmount() {
  //one pulse: 0.33mm rain
  rainAmount += 0.33;
}

float RainSensor::getRainAmount(bool clearVars) {
  float temp = rainAmount;
  if (clearVars)
    rainAmount = 0;
  return temp;
}

