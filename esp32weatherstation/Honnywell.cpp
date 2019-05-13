#include "Honnywell.h"

HonnywellHPM::HonnywellHPM(Stream &_ser):ser(_ser){
  /* Nothing more to do here */
}

void HonnywellHPM::begin(){
  delay(5);             // Set serial mode of HPM sensor to continuous read mode
  ser.print(0x68);
  ser.print(0x01);
  ser.print(0x01);
  ser.print(0x96);
  delay(2000);
  ser.print(0x68);
  ser.print(0x01);
  ser.print(0x40);
  ser.print(0x57);
  delay(1000);
}

void HonnywellHPM::ProcessData(){
   while(ser.available()) {
    hpmDecode((byte) ser.read());
   }
}


void HonnywellHPM::hpmDecode(byte x) 
{
  // Process HPM Sensor serial data, see https://sensing.honeywell.com/honeywell-sensing-hpm-series-particle-sensors-datasheet-32322550-c-en.pdf
  static byte hpmBuffer[32];
  static unsigned int hpmPM25Buffer[20];
  static unsigned int hpmPM10Buffer[20];
  static int posbuffer = 0;
  static byte lastx;
  static unsigned long tmupdate;
  unsigned int checksum = 0;
  
  hpmBuffer[posbuffer] = x;
  posbuffer++;
  if(posbuffer >= 2)
  {
    if(posbuffer >= 32) 
    {
      posbuffer = 0;
      for(int i = 0; i < 30; i++)
      {
        checksum += (unsigned int) hpmBuffer[i];
      }
      if(checksum == (unsigned int) (hpmBuffer[30] <<8) + (unsigned int) hpmBuffer[31])
      {
        hpmPM25 = (unsigned int) (hpmBuffer[6] <<8) + (unsigned int) hpmBuffer[7];
        hpmPM10 = (unsigned int) (hpmBuffer[8] <<8) + (unsigned int) hpmBuffer[9];
        if(hpmPM25 > 0 && hpmPM25 < 65535)
        {
          for(int n = 1; n < 20; n++)
          {
            hpmPM25Buffer[n - 1] = hpmPM25Buffer[n];
            hpmPM10Buffer[n - 1] = hpmPM10Buffer[n];
          }
          hpmPM25Buffer[20 - 1] = hpmPM25;
          hpmPM10Buffer[20 - 1] = hpmPM10;
          unsigned int mn25 = 65535;
          unsigned int mx25 = 0;
          unsigned int mn10 = 65535;
          unsigned int mx10 = 0;
          unsigned long avg25 = 0;
          unsigned long avg10 = 0;
          for(int q = 0; q < 20; q++)
          {
            avg25 += (unsigned long) hpmPM25Buffer[q];
            avg10 += (unsigned long) hpmPM10Buffer[q];
            if(hpmPM25Buffer[q] < mn25)
              mn25 = hpmPM25Buffer[q];
            if(hpmPM25Buffer[q] > mx25)
              mx25 = hpmPM25Buffer[q];
            if(hpmPM10Buffer[q] < mn10)
              mn10 = hpmPM10Buffer[q];
            if(hpmPM10Buffer[q] > mx10)
              mx10 = hpmPM10Buffer[q];
            avgPM25 = (unsigned int) ((avg25 - (unsigned long) mn25 - (unsigned long) mx25)) / 20;
            avgPM10 = (unsigned int) ((avg10 - (unsigned long) mn10 - (unsigned long) mx10)) / 20;
          }
          hpmAge = millis();
        }
      } else {
        ;
      }
    }
  } 
  if(x == 0x4D && lastx == 0x42)
  {
    posbuffer = 2;
    hpmBuffer[0] = 0x42;
    hpmBuffer[1] = 0x4D;
  } 
  lastx = x;
}

bool HonnywellHPM::getData(float *p25, float *p10){
  *p25 = (float)hpmPM25 ;
  *p10 = (float)hpmPM10;
  if( (millis() - hpmAge ) > 2000 ){
    return false;
  } else {
    return true;
  }
}
