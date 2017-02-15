#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <ArduinoJson.h>
#include "FS.h"

#define USE_SERIAL Serial

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

void loop() {
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {
//      checkGPIO(0);
//      checkGPIO(2);
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
void checkGPIO(int gpioIndex) {
    USE_SERIAL.printf("Update GPIO%d \n", gpioIndex);

    String path = "/gpio/gpio"+String(gpioIndex)+".json";
    int value = firebaseGet(path).toInt();

    // httpCode will be negative on error
    if(value >= 0) {
      USE_SERIAL.printf("GPIO%d = %d\n", gpioIndex, value);
      digitalWrite(gpioIndex, value);
    } else {
        USE_SERIAL.println("Failed to update GPIO");
    }
}

void checkConfig() {
  String response = firebaseGet("/setup.json");
  char json[response.length()];
  response.toCharArray(json, response.length());

  StaticJsonBuffer<200> jsonBuffer;  
  JsonObject& setup = jsonBuffer.parseObject(json);
  
  int value = setup["delay"];
  if (value > 0) {
    updateDelay = value;
  }
}

//https://esp-ci.firebaseio.com/setup/wifi
bool saveConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["CSVisitante"] = "#wirelAP54111";

  File configFile = SPIFFS.open("/wifi.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
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

