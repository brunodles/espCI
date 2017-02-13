#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

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
      checkGPIO(0);
      checkGPIO(2);
      checkConfig();
    }

    delay(updateDelay);
    // todo update config file
    // * get More Wifi-setting
    // * get get CI-urls
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
  int value = firebaseGet("/setup/delay.json").toInt();
  if (value > 0) {
    updateDelay = value;  
  }
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

