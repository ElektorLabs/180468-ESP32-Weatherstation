#ifndef __HONNYWHELL_H__
 #define __HONNYWHELL_H__
  #include "Arduino.h"

  class HonnywellHPM{
    private:
      uint32_t hpmAge = 1;   // Time in milliseconds since last valid read out --> ttl = millis() - hpmAge;
      uint16_t hpmPM25 = 0;   // Last read for 2.5 ug/m3
      uint16_t hpmPM10 = 0;   // Last read for 10.0 ug/m3 
      uint16_t avgPM25 = 0;   // Average read for last 20 reads for 2.5 ug/m3
      uint16_t avgPM10 = 0;   // Average read for last 20 reads for 10.0 ug/m3
      
      Stream &ser;

      void hpmDecode(byte x);
    
    public:
      HonnywellHPM(Stream &_ser);
      void begin();
      void ProcessData( void );
      uint32_t GethpmAge( void ); 
      uint16_t GethpmPM25( void );
      uint16_t GethpmPM10( void );
      uint16_t GetavgPM25( void );   
      uint16_t GetavgPM10( void );   
      bool getData(float *p25, float *p10);     
      
  };


#endif
