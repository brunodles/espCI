#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;

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

    WiFiMulti.addAP("Lima", "password");
    // todo Read Config file

}

void loop() {
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {
      checkGPIO(0);
      checkGPIO(2);
    }

    delay(10000);
    // todo update config file
    // * get More Wifi-setting
    // * get get CI-urls
}

void checkGPIO(int gpioIndex) {
    USE_SERIAL.printf("Update GPIO%d \n", gpioIndex);
    
    HTTPClient http;
    String url = "https://esp-ci.firebaseio.com/gpio/gpio"+String(gpioIndex)+".json";
    USE_SERIAL.println(url);
    http.begin(url, "9A:E1:A3:B7:88:E0:C9:A3:3F:13:72:4E:B5:CB:C7:27:41:B2:0F:6A");

    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
      
        if(httpCode == HTTP_CODE_OK) {
            int payload = http.getString().toInt();
            USE_SERIAL.printf("GPIO%d = %d\n", gpioIndex, payload);
            digitalWrite(gpioIndex, payload);
        }
    } else {
        USE_SERIAL.printf("[HTTP] GET... failed, error: %d %s\n", httpCode, http.errorToString(httpCode).c_str());
    }

    http.end();
  
}

