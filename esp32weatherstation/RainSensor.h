#ifndef RAINSENSOR_H
#define RAINSENSOR_H

#include "Arduino.h"

class RainSensor {
  private:
    int pin;
    
    float rainAmount = 0; //in mm
    
  public:
    RainSensor(int _pin);
    void initRainSensor();
    void clearRainAmount();
    void calcRainAmount();
    float getRainAmount(bool clearVars = true);
};

#endif
