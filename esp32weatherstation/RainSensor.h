#ifndef RAINSENSOR_H
#define RAINSENSOR_H

#include "Arduino.h"

class RainSensor {
  private:
    int pin;
    
    typedef struct{
      float rainAmount; //in mm
      uint32_t lastClear;
    } raindata_t;

  raindata_t Datasets[3];

  public:
    RainSensor(int _pin);
    void initRainSensor();
    void calcRainAmount();
    void clearRainAmount(uint32_t idx);
    float getRainAmount(uint32_t idx,bool clearVars = true);
    //float getRainCurrentAmount( void );
};

#endif
