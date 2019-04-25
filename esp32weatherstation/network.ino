//network settings
#include "webfunctions.h"
#include <ESPmDNS.h>
#include <DNSServer.h>

#define DNS_PORT ( 53 )

DNSServer* dnsServer=NULL;

uint8_t macAddr[6];
char stringBufferAP[33]; 
String APSSID = "ESP32 XX:XX:XX";

String ssid;
String pass;

String SSIDList(String separator = ",") {
  Serial.println("Scanning networks");
  String ssidList;
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    Serial.println(String(i) + ": " + ssid);
    if (ssidList.indexOf(ssid) != -1) {
      Serial.println("SSID already in list");
    }
    else {
      if (ssidList != "")
        ssidList += separator;
      ssidList += ssid;
    }
  }
  return ssidList;
}

//send a list of available networks to the client connected to the webserver
void getSSIDList() {
  Serial.println("SSID list requested");
  sendData(SSIDList());
}

//store the wifi settings configured on the webpage and restart the esp to connect to this network
void setWiFiSettings() {
  Serial.println("WiFi settings received");
  Serial.println(server->uri());
  ssid = server->arg("ssid");
  pass = server->arg("pass");
  String response = "Attempting to connect to '" + ssid + "'. The WiFi module restarts and tries to connect to the network.";
  sendData(response);
  Serial.println("Saving network credentials and restart.");
  storeNetworkCredentials();
  delay(1000);
  ESP.restart();
}

//send the wifi settings to the connected client of the webserver
void getWiFiSettings() {
  Serial.println("WiFi settings requested");
  String response;
  response += ssid + ",";
  response += SSIDList(";");
  sendData(response);
}

//store the upload settings configured on the webpage
void setUploadSettings() {
  Serial.println("Upload settings received");
  Serial.println(server->uri());
  thingspeakApi = server->arg("tsApi");
  tsfWindSpeed = server->arg("tsfWS").toInt();
  tsfWindDir = server->arg("tsfWD").toInt();
  tsfRainAmount = server->arg("tsfRA").toInt();
  tsfTemperature = server->arg("tsfT").toInt();
  tsfHumidity = server->arg("tsfH").toInt();
  tsfAirpressure = server->arg("tsfA").toInt();
  tsfPM25 = server->arg("tsfPM25").toInt();
  tsfPM10 = server->arg("tsfPM10").toInt();
  thingspeakEnabled = (server->arg("tsEnabled") == "1");
  senseBoxStationId = server->arg("sbStationId");
  senseBoxWindSId = server->arg("sbWindSId");
  senseBoxWindDId = server->arg("sbWindDId");
  senseBoxRainId = server->arg("sbRainId");
  senseBoxTempId = server->arg("sbTempId");
  senseBoxHumId = server->arg("sbHumId");
  senseBoxPressId = server->arg("sbPressId");
  senseBoxPM25Id = server->arg("sbPM25Id");
  senseBoxPM10Id = server->arg("sbPM10Id");
  senseBoxEnabled = (server->arg("sbEnabled") == "1");
  long recvInterval = server->arg("interval").toInt();
  uploadInterval = ((recvInterval <= 0) ? hourMs : (recvInterval * 60 * 1000)); //convert to ms (default is 1 hr)
  storeUploadSettings();
  sendData("Upload settings stored");
}

void getUploadSettings() {
  Serial.println("Upload settings requested");
  String response;
  response += thingspeakApi + ",";
  response += String(tsfWindSpeed) + ",";
  response += String(tsfWindDir) + ",";
  response += String(tsfRainAmount) + ",";
  response += String(tsfTemperature) + ",";
  response += String(tsfHumidity) + ",";
  response += String(tsfAirpressure) + ",";
  response += String(tsfPM25) + ",";
  response += String(tsfPM10) + ",";
  response += String(thingspeakEnabled ? "1" : "0") + ",";
  response += senseBoxStationId + ",";
  response += senseBoxWindSId + ",";
  response += senseBoxWindDId + ",";
  response += senseBoxRainId + ",";
  response += senseBoxTempId + ",";
  response += senseBoxHumId + ",";
  response += senseBoxPressId + ",";
  response += senseBoxPM25Id + ",";
  response += senseBoxPM10Id + ",";
  response += String(senseBoxEnabled ? "1" : "0") + ",";
  response += String(uploadInterval / 1000 / 60); //convert to minutes
  sendData(response);
}

//send the weather data to the connected client of the webserver
void getWeatherData() {
  Serial.println("Weather data requested");
  String response;
  response += String(windSpeed*3.6) + ","; //km/h
  response += String(beaufort) + " (" + beaufortDesc + "),";
  response += String(windDir) + ",";
  response += String(temperature) + ",";
  response += String(humidity) + ",";
  response += String(pressure) + ",";
  response += String(PM25) + ",";
  response += String(PM10) +",";
  response += String(rainAmountAvg);
  sendData(response);
}

//send the battery data to the connected client of the webserver
void getBatteryData() {
  Serial.println("Battery data requested");
  String response;
  response += String(batteryVoltage) + ",";
  response += String(batteryCharging ? "1" : "0");
  sendData(response);
}

//toggle the charging of the battery
void toggleCharging() {
  Serial.println("Toggle charging requested");
  batteryCharging = !batteryCharging;
  sendData("Battery charging " + String(batteryCharging ? "started" : "stopped"));
}

//restart the esp as requested on the webpage
void restart() {
  sendData("The ESP32 will restart and you will be disconnected from the '" + APSSID + "' network.");
  delay(1000);
  ESP.restart();
}

//get the content type of a filename
String getContentType(String filename) {
  if (server->hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//send a file from the SPIFFS to the connected client of the webserver
void sendFile() {
  String path = server->uri();
  Serial.println("Got request for: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (SPIFFS.exists(path)) {
    Serial.println("File " + path + " found");
    File file = SPIFFS.open(path, "r");
    server->streamFile(file, contentType);
    file.close();
  }
  else {
    Serial.println("File '" + path + "' doesn't exist");
    server->send(404, "text/plain", "The requested file doesn't exist");
  }
  
  lastAPConnection = millis();
}

//send data to the connected client of the webserver
void sendData(String data) {
  Serial.println("Sending: " + data);
  server->send(200, "text/plain", data);
  
  lastAPConnection = millis();
}

//initialize wifi by connecting to a wifi network or creating an accesspoint
void initWiFi() {
  digitalWrite(APLed, LOW);
  digitalWrite(STALed, LOW);
  Serial.print("WiFi: ");
  if (!digitalRead(APPin)) {
    Serial.println("AP");
    configureSoftAP();
  }
  else {
    Serial.println("STA");
    if (!connectWiFi()) {
      Serial.println("Connecting failed. Starting AP");
      configureSoftAP();
    }
    else {
      configureServer();
      configureClient();
      digitalWrite(STALed, HIGH);
    }
  }
}

//connect the esp32 to a wifi network
bool connectWiFi() {
  if (ssid == "") {
    Serial.println("SSID unknown");
    return false;
  }
  WiFi.mode(WIFI_STA);
  Serial.println("Attempting to connect to " + ssid + ", pass: " + pass);
  WiFi.begin(ssid.c_str(), pass.c_str());
  for (int timeout = 0; timeout < 15; timeout++) { //max 15 seconds
    int status = WiFi.status();
    if ((status == WL_CONNECTED) || (status == WL_CONNECT_FAILED) || (status == WL_NO_SSID_AVAIL))
      break;
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to " + ssid);
    Serial.println("WiFi status: " + WiFiStatusToString());
    WiFi.disconnect();
    return false;
  }
  Serial.println("Connected to " + ssid);
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  
  return true;
}

//configure the access point of the esp32
void configureSoftAP() {
 
  WiFi.mode(WIFI_AP);
  WiFi.softAPmacAddress(macAddr);
  snprintf(stringBufferAP,32,"ESP32 Weatherstation %02x:%02x:%02x",macAddr[3],macAddr[4],macAddr[5]);
  APSSID = stringBufferAP;
  Serial.println("Configuring AP: " + String(APSSID));
  WiFi.softAP(APSSID.c_str(), NULL, 1, 0, 4);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(ip);
  lastAPConnection = millis();
  digitalWrite(APLed, HIGH);

  dnsServer = new DNSServer();
  dnsServer->start(DNS_PORT, "*", ip);

  /* Setup MDNS */
  if (!MDNS.begin("Weatherstation")) {
    Serial.println("Start MDNS: fail");   
  } else { 
    Serial.println("Start MDNS: okay");   
  }
  
  configureServer();

}

//initialize the webserver on port 80
void configureServer() {
  server = new WebServer(80);
  server->on("/getWeatherData", HTTP_GET, getWeatherData);
  server->on("/getBatteryData", HTTP_GET, getBatteryData);
  server->on("/toggleCharging", HTTP_GET, toggleCharging);
  server->on("/setWiFiSettings", HTTP_GET, setWiFiSettings);
  server->on("/getWiFiSettings", HTTP_GET, getWiFiSettings);
  server->on("/setUploadSettings", HTTP_GET, setUploadSettings);
  server->on("/getUploadSettings", HTTP_GET, getUploadSettings);
  server->on("/getSSIDList", HTTP_GET, getSSIDList);
  server->on("/restart", HTTP_GET, restart);
  register_functions(server);
  server->onNotFound(sendFile); //handle everything except the above things
  server->begin();
  Serial.println("Webserver started");
}

void configureClient() {
  client = new WiFiClient();
  clientS = new WiFiClientSecure();
  clientS->setCACert(certificate);
}

//request the specified url of the specified host
String performRequest(bool secure, String host, String url, int port = 80, String method = "GET", String headers = "Connection: close\r\n", String data = "") {
  WiFiClient* c = (secure ? clientS : client);
  Serial.println("Connecting to host '" + host + "' on port " + String(port));
  c->connect(host.c_str(), port); //default ports: http: port 80, https: 443
  String request = method + " " + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   headers + "\r\n";
  Serial.println("Requesting url: " + request);
  c->print(request);
  if (data != "") {
    Serial.println("Data: " + data);
    c->print(data + "\r\n");
  }
  
  unsigned long timeout = millis();
  while (c->available() == 0) {
    if (timeout + 5000 < millis()) {
      Serial.println("Client timeout");
      c->stop();
      return "";
    }
  }
  //read client reply
  String response;
  while(c->available()) {
    response = c->readStringUntil('\r');
  }
  Serial.println("Response: " + response);
  c->stop();
  return response;
}

String WiFiStatusToString() {
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:     return "IDLE"; break;
    case WL_NO_SSID_AVAIL:   return "NO SSID AVAIL"; break;
    case WL_SCAN_COMPLETED:  return "SCAN COMPLETED"; break;
    case WL_CONNECTED:       return "CONNECTED"; break;
    case WL_CONNECT_FAILED:  return "CONNECT_FAILED"; break;
    case WL_CONNECTION_LOST: return "CONNECTION LOST"; break;
    case WL_DISCONNECTED:    return "DISCONNECTED"; break;
    case WL_NO_SHIELD:       return "NO SHIELD"; break;
    default:                 return "Undefined: " + String(WiFi.status()); break;
  }
}

void NetworkTask( void ){
  //handle WiFi
  if (server != NULL){
    server->handleClient();
  }
  
  if(dnsServer!=NULL){
     dnsServer->processNextRequest();
  }
  
}
