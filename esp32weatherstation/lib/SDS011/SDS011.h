#ifndef SDS011_H
#define SDS011_H

#include "Arduino.h"

//#define SDS_DEBUG

#define SDS_SEND_DATA_LENGTH 19
#define SDS_RECV_DATA_LENGTH 10
#define SDS_READ_TIMEOUT 1000 //ms

#define SDS_CMD_SET_MODE 2
#define SDS_CMD_QUERY_DATA 4
#define SDS_CMD_SET_DEVICE_ID 5
#define SDS_CMD_SET_SLEEP 6
#define SDS_CMD_CHECK_FIRMWARE_VERSION 7
#define SDS_CMD_SET_WORKING_PERIOD 8

#define SDS_RECCMD 0xC5
#define SDS_RECCMD_QUERY 0xC0

#define SDS_GET_MODE 0
#define SDS_SET_MODE 1

#define SDS_SET_CONTINUOUS 0
#define SDS_SET_QUERY 1
#define SDS_SET_SLEEP 0
#define SDS_SET_WORK 1

class SDS011 {
  private:
    Stream &ser;
    byte queryId[2] = {0xff, 0xff};
    bool reportMode = SDS_SET_CONTINUOUS; //factory default
    void sendData(byte *buf, int len);
    bool receiveData(byte *buf);
    bool sendCommand(byte *buf, int len, byte *recBuf);
  
  public:
    SDS011(Stream &_ser);
    void getQueryId(byte *buf);
    void setQueryId(byte *buf);
    bool getMode(bool *m);
    bool setMode(bool m);
    bool getData(float *p25, float *p10);
    bool setDeviceId(byte *id);
    bool getSleepMode(bool *m);
    bool setSleepMode(bool m);
    bool getWorkingPeriod(int *per);
    bool setWorkingPeriod(int period);
    bool getFirmwareVersion(byte *ver);
};

void printBuf(byte *buf, int len);
  
#endif
  