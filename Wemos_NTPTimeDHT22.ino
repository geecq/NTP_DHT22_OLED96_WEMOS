
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h> 
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "time.h" 

#define SCREEN_WIDTH 128      // OLED display width, in pixels
#define SCREEN_HEIGHT 64      // OLED display height, in pixels
#define DHTPIN D4             // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT22      // DHT 22 (AM2302)

/* Uses hardware SPI connections */
#define OLED_DC     D2
#define OLED_CS     D8
#define OLED_RESET  D1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);
DHT_Unified dht(DHTPIN, DHTTYPE);

const char* ssid     = "Your-SSID";
const char* password = "Your-PASSWORD";
const char* Timezone = "PST8PDT,M3.2.0,M11.1.0"; // posix PST time zone code      
String Time_format;  
float temp, rh;
String Date_str, Time_str;

void setup() {
  Serial.begin(115200);
  StartWiFi();
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", Timezone, 1);
  Time_format = "M"; 
  UpdateLocalTime(Time_format);
  display.begin(SSD1306_SWITCHCAPVCC);
  
  /* Get a first reading from DHT22 */
  display.clearDisplay();
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  temp = (event.temperature * 9/5) + 32;
  dht.humidity().getEvent(&event);
  rh = event.relative_humidity;
  
  /* Print to OLED display */
  display.clearDisplay();
  display.setTextSize(2);             
  display.setTextColor(WHITE);       
  display.setCursor(0,0);             
  display.println(Date_str);
  display.println(Time_str);
  display.print(temp);
  display.println(F(" F"));
  display.print(rh);
  display.println(F("% RH"));
  display.display(); 
}

void loop() {
  ArduinoOTA.handle();
  static unsigned long lastMillis = millis();
  static int DHT_delay = 0;
  if (millis() - lastMillis >= 1000) { // time update every 1 sec
    UpdateLocalTime(Time_format);
    lastMillis = millis();
    DHT_delay++;
  }
  if (DHT_delay >= 9) { // temp & humidity reading every 10 secs
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  temp = (event.temperature * 9/5) + 32;
  dht.humidity().getEvent(&event);
  rh = event.relative_humidity;
  DHT_delay = 0;
  } 
  display.clearDisplay();
  display.setTextSize(2);             
  display.setTextColor(WHITE);       
  display.setCursor(0,0);             
  display.println(Date_str);
  display.println(Time_str);
  display.print(temp);
  display.println(F(" F"));
  display.print(rh);
  display.println(F("% RH"));
  display.display(); 
}

void UpdateLocalTime(String Format){
  time_t now;
  time(&now);
  char hour_output[30], day_output[30];
  strftime(day_output, 30, "%a %d", localtime(&now)); 
  strftime(hour_output, 30, "%T", localtime(&now));   
  Date_str = day_output;
  Time_str = hour_output;
}

void StartWiFi(){
  WiFi.mode(WIFI_STA);
  Serial.println("Booting");
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    //Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
