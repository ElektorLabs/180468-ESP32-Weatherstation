#include "RainSensor.h"

/* This class is modified to support three sinks for data with different intervalls */

RainSensor::RainSensor(int _pin) {
  pin = _pin;
}

void RainSensor::initRainSensor() {
  pinMode(pin, INPUT);
  for(uint32_t i=0;i< (sizeof(Datasets) / sizeof(Datasets[0]) );i++){
    Datasets[i].lastClear=0;
    Datasets[i].rainAmount=0;
  }
}

void RainSensor::clearRainAmount(uint32_t idx) {
  if(idx < (sizeof(Datasets) / sizeof(Datasets[0]) ) ){
    Datasets[idx].rainAmount = 0;
  }
  
}

void RainSensor::calcRainAmount() {
  //one pulse: 0.33mm rain
  for(uint32_t i=0;i< (sizeof(Datasets) / sizeof(Datasets[0]) );i++){
    Datasets[i].rainAmount += 0.33;  
  }
  
}

float RainSensor::getRainAmount(uint32_t idx, bool clearVars) {
  float temp = 0;
  if(idx < (sizeof(Datasets) / sizeof(Datasets[0]) ) ){
    temp=Datasets[idx].rainAmount ;
    if (clearVars){
      Datasets[idx].rainAmount = 0;
      Datasets[idx].lastClear=millis();
    }
  }
  return temp;
}

