#ifndef RAINSENSOR_H
#define RAINSENSOR_H

#include "Arduino.h"

class RainSensor {
  private:
    int pin;
    
    float rainAmount = 0; //in mm
    uint32_t lastClear=0;
  public:
    RainSensor(int _pin);
    void initRainSensor();
    void clearRainAmount();
    void calcRainAmount();
    float getRainAmount(bool clearVars = true);
    float getRainCurrentAmount( void );
};

#endif
