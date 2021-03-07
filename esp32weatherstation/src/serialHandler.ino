void checkSerial() {
  while (Serial.available()) {
    char in = Serial.read();
    serialIn += in;
    if (in == '\n')
      serialRdy = true;
  }
  handleSerial();
}

void handleSerial() {
  if (serialRdy) {
    Serial.println(serialIn);
    if (serialIn.indexOf("thingspeakUpload") != -1) {
      if (uploadToThingspeak())
        Serial.println("Success");
      else
        Serial.println("Failed");
    }
    else if (serialIn.indexOf("senseBoxUpload") != -1) {
      if (uploadToSenseBox())
        Serial.println("Success");
      else
        Serial.println("Failed");
    }
    else if (serialIn.indexOf("wifiStatus") != -1) {
      Serial.println("WiFi status: " + WiFiStatusToString());
    }
    else if (serialIn.indexOf("printData") != -1) {
      Serial.println("Wind speed:         " + String(ws.getWindSpeed()) + "m/s, " + String(ws.getWindSpeed() * 3.6) + "km/h");
      Serial.println("Beaufort:           " + String(ws.getBeaufort()) + " (" + ws.getBeaufortDesc() + ")");
      Serial.println("Wind speed avg:     " + String(ws.getWindSpeedAvg(false)));
      Serial.println("Wind direction:     " + ws.getWindDirString() + " (" + String(ws.getWindDir()) + ")");
      Serial.println("Wind direction avg: " + String(ws.getWindDirAvg(false)));
      Serial.println("Rain amount:        " + String(rs.getRainAmount(false)) + "mm");
      Serial.println("Temperature:        " + String(temperature) + "*C");
      Serial.println("Humidity:           " + String(humidity) + "%");
      Serial.println("Pressure:           " + String(pressure) + "hPa");
      Serial.println("Dust 10um:          " + String(PM10) + "ug/m3");
      Serial.println("Dust 2.5um:         " + String(PM25) + "ug/m3");
    }
    else if (serialIn.indexOf("WifiSettings") != -1) {
      if (serialIn.indexOf("set") != -1) {
        ssid = serialIn.substring(serialIn.indexOf(":") + 1, serialIn.indexOf(","));
        pass = serialIn.substring(serialIn.indexOf(",") + 1, serialIn.indexOf("\n"));
        storeNetworkCredentials();
      }
      Serial.println("WiFi settings: SSID: " + ssid + ", pass: " + pass);
    }
    else if (serialIn.indexOf("thingspeakSettings") != -1) {
      Serial.println("Thingspeak settings:");
      Serial.println("  Api key:  " + thingspeakApi);
      Serial.println("  Field numbers:");
      Serial.println("    Wind speed:     " + String(tsfWindSpeed));
      Serial.println("    Wind direction: " + String(tsfWindDir));
      Serial.println("    Rain amount:    " + String(tsfRainAmount));
      Serial.println("    Temperature:    " + String(tsfTemperature));
      Serial.println("    Humidity:       " + String(tsfHumidity));
      Serial.println("    Airpressure:    " + String(tsfAirpressure));
      Serial.println("    PM25:           " + String(tsfPM25));
      Serial.println("    PM10:           " + String(tsfPM10));
      Serial.println("  Enabled:  " + String(thingspeakEnabled ? "Yes" : "No"));
      Serial.println("  Interval: " + String(uploadInterval) + "ms (" + String(uploadInterval/1000/60) + " minutes)");
    }
    else if (serialIn.indexOf("senseBoxSettings") != -1) {
      Serial.println("SenseBox settings:");
      Serial.println("  Station ID:     " + senseBoxStationId);
      Serial.println("  Wind speed ID:  " + senseBoxWindSId);
      Serial.println("  Wind dir ID:    " + senseBoxWindDId);
      Serial.println("  Rain ID:        " + senseBoxRainId);
      Serial.println("  temperature ID: " + senseBoxTempId);
      Serial.println("  Humidity ID:    " + senseBoxHumId);
      Serial.println("  Pressure ID:    " + senseBoxPressId);
      Serial.println("  Enabled:        " + String(senseBoxEnabled ? "Yes" : "No"));
      Serial.println("  Interval: " + String(uploadInterval) + "ms (" + String(uploadInterval/1000/60) + " minutes)");
    }
    else if (serialIn.indexOf("restart") != -1) {
      ESP.restart();
    }
    else if (serialIn.indexOf("timeActive") != -1) {
      unsigned long time = millis();
      Serial.println("Time active: " + String(time) + "ms, " + String(time/1000/60) + " minutes");
    }
    
    serialRdy = false;
    serialIn = "";
  }
}

