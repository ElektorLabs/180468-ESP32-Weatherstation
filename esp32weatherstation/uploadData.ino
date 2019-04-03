//upload the sensor data to thingspeak
bool uploadToThingspeak() {
  Serial.println("Uploading to thingspeak");
  if (thingspeakApi == "") {
    Serial.println("Thingspeak API key not set");
    return false;
  }
  
  printUploadValues();
  
  String thingspeakHost = "api.thingspeak.com";
  String thingspeakUrl = "/update";
  thingspeakUrl += "?api_key=" + thingspeakApi;
  if (tsfWindSpeed != 0)
    thingspeakUrl += "&field" + String(tsfWindSpeed) + "=" + String(windSpeedAvg*3.6); //km/h
  if (tsfWindDir != 0)
    thingspeakUrl += "&field" + String(tsfWindDir) + "=" + String(windDirAvg);
  if (tsfRainAmount != 0)
    thingspeakUrl += "&field" + String(tsfRainAmount) + "=" + String(rainAmountAvg);
  if (tsfTemperature != 0)
    thingspeakUrl += "&field" + String(tsfTemperature) + "=" + String(temperature);
  if (tsfHumidity != 0)
    thingspeakUrl += "&field" + String(tsfHumidity) + "=" + String(humidity);
  if (tsfAirpressure != 0)
    thingspeakUrl += "&field" + String(tsfAirpressure) + "=" + String(pressure);
  if (tsfPM25 != 0)
    thingspeakUrl += "&field" + String(tsfPM25) + "=" + String(PM25);
  if (tsfPM10 != 0)
    thingspeakUrl += "&field" + String(tsfPM10) + "=" + String(PM10);

  String resp = performRequest(false, thingspeakHost, thingspeakUrl);
  return (resp != "" && !resp.startsWith("0"));
}

//upload the sensor values to senseBox
bool uploadToSenseBox() {
  Serial.println("Uploading to SenseBox");
  if (senseBoxStationId == "") {
    Serial.println("SenseBox station ID not set");
    return false;
  }
  
  printUploadValues();
  
  String csv;
  if (senseBoxWindSId != "")
    csv += senseBoxWindSId + "," + String(windSpeedAvg*3.6) + "\r\n"; //km/h
  if (senseBoxWindDId != "")
    csv += senseBoxWindDId + "," + String(windDirAvg) + "\r\n";
  if (senseBoxRainId != "")
    csv += senseBoxRainId + "," + String(rainAmountAvg) + "\r\n";
  if (senseBoxTempId != "")
    csv += senseBoxTempId + "," + String(temperature) + "\r\n";
  if (senseBoxHumId != "")
    csv += senseBoxHumId + "," + String(humidity) + "\r\n";
  if (senseBoxPressId != "")
    csv += senseBoxPressId + "," + String(pressure) + "\r\n";
  if (senseBoxPM25Id != "")
    csv += senseBoxPM25Id + "," + String(PM25) + "\r\n";
  if (senseBoxPM10Id != "")
    csv += senseBoxPM10Id + "," + String(PM10) + "\r\n";

  if (csv == "") {
    Serial.println("Sensor API keys not set");
    return false;
  }
  
  String senseBoxHost = "api.opensensemap.org";
  String senseBoxUrl = "/boxes/" + senseBoxStationId + "/data";
  String headers = "Content-Type: text/csv\r\n";
  headers += "Connection: close\r\n";
  headers += "Content-Length: " + String(csv.length()) + "\r\n";

  String resp = performRequest(true, senseBoxHost, senseBoxUrl, 443, "POST", headers, csv);
  return true;
}

void printUploadValues() {
  Serial.println("Values:");
  Serial.println("WindSpeedAvg:  " + String(windSpeedAvg));
  Serial.println("WindDirAvg:    " + String(windDirAvg));
  Serial.println("RainAmountAvg: " + String(rainAmountAvg));
  Serial.println("Temperature:   " + String(temperature));
  Serial.println("Humidity:      " + String(humidity));
  Serial.println("Pressure:      " + String(pressure));
  Serial.println("PM25:          " + String(PM25));
  Serial.println("PM10:          " + String(PM10));
}

