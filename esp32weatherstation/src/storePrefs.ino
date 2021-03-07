/*
 * Note: preference names can't be longer than 15 characters
 */
void loadNetworkCredentials() {
  pref.begin("weather", false);
  ssid = pref.getString("ssid");
  pass = pref.getString("pass");
  pref.end();
}

void storeNetworkCredentials() {
  pref.begin("weather", false);
  pref.putString("ssid", ssid);
  pref.putString("pass", pass);
  pref.end();
}

void loadUploadSettings() {
  pref.begin("weather", false);
  thingspeakEnabled = pref.getBool("thingspeakEn");
  thingspeakApi = pref.getString("thingspeakApi");
  tsfWindSpeed = pref.getInt("tsfWindSpeed");
  tsfWindDir = pref.getInt("tsfWindDir");
  tsfRainAmount = pref.getInt("tsfRainAmount");
  tsfTemperature = pref.getInt("tsfTemperature");
  tsfHumidity = pref.getInt("tsfHumidity");
  tsfAirpressure = pref.getInt("tsfAirpressure");
  tsfPM25 = pref.getInt("tsfPM25");
  tsfPM10 = pref.getInt("tsfPM10");

  senseBoxEnabled = pref.getBool("sbEnabled");
  senseBoxStationId = pref.getString("sbStationId");
  senseBoxTempId = pref.getString("sbTempId");
  senseBoxHumId = pref.getString("sbHumId");
  senseBoxPressId = pref.getString("sbPressId");
  senseBoxWindSId = pref.getString("sbWindSId");
  senseBoxWindDId = pref.getString("sbWindDId");
  senseBoxRainId = pref.getString("sbRainId");
  senseBoxPM25Id = pref.getString("sbPM25Id");
  senseBoxPM10Id = pref.getString("sbPM10Id");
  
  uploadInterval = ((pref.getUInt("uploadInterval") != 0) ? pref.getUInt("uploadInterval") : hourMs);
  pref.end();
}

void storeUploadSettings() {
  pref.begin("weather", false);
  pref.putBool("thingspeakEn", thingspeakEnabled);
  pref.putString("thingspeakApi", thingspeakApi);
  pref.putInt("tsfWindSpeed", tsfWindSpeed);
  pref.putInt("tsfWindDir", tsfWindDir);
  pref.putInt("tsfRainAmount", tsfRainAmount);
  pref.putInt("tsfTemperature", tsfTemperature);
  pref.putInt("tsfHumidity", tsfHumidity);
  pref.putInt("tsfAirpressure", tsfAirpressure);
  pref.putInt("tsfPM25", tsfPM25);
  pref.putInt("tsfPM10", tsfPM10);

  pref.putBool("sbEnabled", senseBoxEnabled);
  pref.putString("sbStationId", senseBoxStationId);
  pref.putString("sbTempId", senseBoxTempId);
  pref.putString("sbHumId", senseBoxHumId);
  pref.putString("sbPressId", senseBoxPressId);
  pref.putString("sbWindSId", senseBoxWindSId);
  pref.putString("sbWindDId", senseBoxWindDId);
  pref.putString("sbRainId", senseBoxRainId);
  pref.putString("sbPM25Id", senseBoxPM25Id);
  pref.putString("sbPM10Id", senseBoxPM10Id);
  
  pref.putUInt("uploadInterval", uploadInterval);
  pref.end();
}

