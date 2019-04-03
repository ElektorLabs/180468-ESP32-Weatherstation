#include "sds011.h"

SDS011::SDS011(Stream &_ser):ser(_ser) {
}

void SDS011::sendData(byte *buf, int len) {
  byte cmd[SDS_SEND_DATA_LENGTH] = {0xAA, 0xB4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, queryId[0], queryId[1], 0, 0xAB};

  //add data
  for (int i = 0; i < min(len, 13); i++)
    cmd[2 + i] = buf[i];
  
  //calculate checksum
  for (int i = 0; i < 15; i++) 
    cmd[17] += cmd[2 + i];

  for (int i = 0; i < SDS_SEND_DATA_LENGTH; i++)
    ser.write(cmd[i]);

  #ifdef SDS_DEBUG
    Serial.print("sending: ");
    printBuf(cmd, SDS_SEND_DATA_LENGTH);
  #endif
}

//receive data starting with 0xAA and store this in buf
bool SDS011::receiveData(byte *buf) {
  bool start = false;
  int nr = 0;
  unsigned long startRead = millis();
  byte in = 0;
  while (nr < SDS_RECV_DATA_LENGTH) {
    if (ser.available()) {
      in = ser.read();
      if (in == 0xAA)
        start = true;
      buf[nr] = in;
      
      if (start)
        nr++;
    }
    if ((startRead + SDS_READ_TIMEOUT) < millis()) //if the timeout time ha passed
      return false;
  }

  #ifdef SDS_DEBUG
    Serial.print("receiving: ");
    printBuf(buf, SDS_RECV_DATA_LENGTH);
  #endif

  //check last byte of buffer
  if (buf[SDS_RECV_DATA_LENGTH-1] != 0xAB)
    return false;

  //check checksum
  byte recChk = 0;
  for (int i = 2; i < SDS_RECV_DATA_LENGTH-2; i++)
    recChk += buf[i];
  #ifdef SDS_DEBUG
    Serial.println("Checksum: " + String(recChk) + ", should be: " + String(buf[SDS_RECV_DATA_LENGTH-2]));
  #endif
  return recChk == buf[SDS_RECV_DATA_LENGTH-2];
}

//send a command to the sds011 and receive the data it sends back
bool SDS011::sendCommand(byte *buf, int len, byte *recBuf) {
  ser.flush(); //clear the buffer
  sendData(buf, len);
  int attempts = 3;
  bool s = false;
  while (attempts > 0) { //retry three times to receive the requested data
    s = receiveData(recBuf);
    //check if the received data is the one which was requested
    if (((buf[0] == SDS_CMD_QUERY_DATA) && (recBuf[1] == SDS_RECCMD_QUERY)) || ((buf[0] != SDS_CMD_QUERY_DATA) && (recBuf[2] == buf[0])))
      return s;
    attempts--;
    delay(50);
  }
  return false;
}

//get the ID of the sensor of which data should be requested
void SDS011::getQueryId(byte *buf) {
  buf[0] = queryId[0];
  buf[1] = queryId[1];
}

//set the ID of the sensor of which data should be requested
void SDS011::setQueryId(byte *buf) {
  queryId[0] = buf[0];
  queryId[1] = buf[1];
}

//get the current mode of the sensor (continuous or query)
bool SDS011::getMode(bool *m) {
  byte buf[2] = {SDS_CMD_SET_MODE, SDS_GET_MODE};
  byte recBuf[SDS_RECV_DATA_LENGTH] = {0};
  bool s = sendCommand(buf, 2, recBuf);
  *m = recBuf[4];
  return s;
}

//set the current mode of the sensor (continuous or query)
bool SDS011::setMode(bool m) {
  byte buf[3] = {SDS_CMD_SET_MODE, SDS_SET_MODE, m};
  byte recBuf[SDS_RECV_DATA_LENGTH] = {0};
  bool s = sendCommand(buf, 3, recBuf);
  reportMode = m;
  return s;
}

//get the measurements of the sensor
bool SDS011::getData(float *p25, float *p10) {
  byte buf[1] = {SDS_CMD_QUERY_DATA};
  byte recBuf[SDS_RECV_DATA_LENGTH] = {0};
  bool s = sendCommand(buf, 1, recBuf);
  int ip25 = recBuf[2] | (recBuf[3] << 8);
  int ip10 = recBuf[4] | (recBuf[5] << 8);
  *p25 = (float)ip25 / 10;
  *p10 = (float)ip10 / 10;
  return s;
}

bool SDS011::setDeviceId(byte *id) {
  byte buf[13] = {SDS_CMD_SET_DEVICE_ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, id[0], id[1]};
  byte recBuf[SDS_RECV_DATA_LENGTH] = {0};
  bool s = sendCommand(buf, 13, recBuf);
  return s;
}

//get the sleep mode of the sensor
bool SDS011::getSleepMode(bool *m) {
  byte buf[2] = {SDS_CMD_SET_SLEEP, SDS_GET_MODE};
  byte recBuf[SDS_RECV_DATA_LENGTH] = {0};
  bool s = sendCommand(buf, 2, recBuf);
  *m = recBuf[4];
  return s;
}

//set the sleep mode of the sensor
bool SDS011::setSleepMode(bool m) {
  byte buf[3] = {SDS_CMD_SET_SLEEP, SDS_SET_MODE, m};
  byte recBuf[SDS_RECV_DATA_LENGTH] = {0};
  bool s = sendCommand(buf, 3, recBuf);
  return s;
}

//get the working period of the sensor (0 is continuous, 1 - 30 min is the measurement interval in minutes)
bool SDS011::getWorkingPeriod(int *per) {
  byte buf[2] = {SDS_CMD_SET_WORKING_PERIOD, SDS_GET_MODE};
  byte recBuf[SDS_RECV_DATA_LENGTH] = {0};
  bool s = sendCommand(buf, 2, recBuf);
  *per = recBuf[4];
  return s;
}

//set the working period of the sensor
bool SDS011::setWorkingPeriod(int period) {
  byte buf[3] = {SDS_CMD_SET_WORKING_PERIOD, SDS_SET_MODE, (byte)period};
  byte recBuf[SDS_RECV_DATA_LENGTH] = {0};
  bool s = sendCommand(buf, 3, recBuf);
  return s;
}

//get the firmware version of the sensor
bool SDS011::getFirmwareVersion(byte *ver) {
  byte buf[1] = {SDS_CMD_CHECK_FIRMWARE_VERSION};
  byte recBuf[SDS_RECV_DATA_LENGTH] = {0};
  bool s = sendCommand(buf, 1, recBuf);
  ver[0] = recBuf[3];
  ver[1] = recBuf[4];
  ver[2] = recBuf[5];
  return s;
}

void printBuf(byte *buf, int len) {
  for (int i = 0; i < len; i++) {
    Serial.print("0x");
    Serial.print(buf[i], HEX);
    if (i < len-1)
      Serial.print(", ");
  }
  Serial.println();
}

