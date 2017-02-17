#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <ArduinoJson.h>
#include "FS.h"

#define USE_SERIAL Serial
#define SETUP "/setup.json"

#define FIREBASE_URL "https://esp-ci.firebaseio.com"
#define FIREBASE_FINGERPRINT "9A:E1:A3:B7:88:E0:C9:A3:3F:13:72:4E:B5:CB:C7:27:41:B2:0F:6A"

ESP8266WiFiMulti WiFiMulti;

int updateDelay = 10000;

void setup() {

  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(0, 0);
  digitalWrite(2, 0);

    USE_SERIAL.begin(115200);
   // USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFiMulti.addAP("Lima", "pass");
    WiFiMulti.addAP("CSVisitante", "pass");
    // todo Read Config file

}

bool loadConfig() {
  File configFile = SPIFFS.open(SETUP, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& setup = jsonBuffer.parseObject(buf.get());

  if (!setup.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  JsonObject& wifiList = setup["wifi"];
  for (JsonObject::iterator it=wifiList.begin(); it!=wifiList.end(); ++it) {
    WiFiMulti.addAP(it->key, it->value.asString());
    Serial.printf("Add wifi %s\n", it->key);
  }

  int value = setup["delay"];
  if (value > 0) {
    updateDelay = value;
  }
  
  return true;
}

void loop() {
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {
      checkGPIO();
      checkConfig();
    }

    delay(updateDelay);
    // todo update config file
    // * get More Wifi-setting
    // * get get CI-urls
    checkConfig();
}

void checkGPIO() {
  String json = firebaseGet("/gpio.json");
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  digitalWrite(0, root["gpio0"]);
  digitalWrite(2, root["gpio2"]);
  
}

void checkConfig() {
  String response = firebaseGet(SETUP);
  char json[response.length()];
  response.toCharArray(json, response.length());

  StaticJsonBuffer<200> jsonBuffer;  
  JsonObject& setup = jsonBuffer.parseObject(json);
  
  int value = setup["delay"];
  if (value > 0) {
    updateDelay = value;
  }
  saveFile(response, SETUP);
}

//https://esp-ci.firebaseio.com/setup/wifi
bool saveFile(String content, String fileName) {
  File file = SPIFFS.open(fileName, "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  file.print(content);
  return true;
}

String firebaseGet(String path) {    
  String url = FIREBASE_URL+path;
  USE_SERIAL.print("GET ");
  USE_SERIAL.println(url);
  HTTPClient http;
  http.begin(url, FIREBASE_FINGERPRINT);
  
  int httpCode = http.GET();
  String payload;
//  if(httpCode > 0) {
  if(httpCode == HTTP_CODE_OK) {
      payload = http.getString();
//    }
  } else {
    payload = "";
  }
  http.end();
  return payload;  
}

