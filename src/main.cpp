#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <LiquidCrystal_I2C.h>
#include <LittleFS.h>
#include <Wire.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "secrets.h"

#define FILAMENT_SCALE // enable FILAMENT_SCALE component
#define SERVER
#define SDA 0x00
#define SCL 0x02
#define RX_PIN 0x03
#define TX_PIN 0x01
// prototypes
void loadConfig();
void saveConfig();
void calibrateScale();
void countdown(int millis);
void showWeight();
void startOTA();
void tareScale();
void startServer();
void setup();
void updateWeb();

LiquidCrystal_I2C lcd(0x27, 16,2);

class Task {
private:
  unsigned long lastRun;
  unsigned int interval;
  // std::function<void()> actualTask_;
  void (*_actualTask_)();

public:
Task(unsigned long lastRun, unsigned int interval, void (func)()){
    this->lastRun = lastRun;
    this->interval = interval;
    this->_actualTask_ = func;
}
  // Task(unsigned long lastRun, unsigned int interval,const std::function<void()> &functionToRun): actualTask_(functionToRun) {
  //   this->lastRun = lastRun;
  //   this->interval = interval;
  // };
  void run() {
    this->lastRun = millis();
    this->_actualTask_();
  }
  bool isReady() {
    return ((millis() - this->lastRun) > this->interval) ? true : false;
  }
};

#ifdef FILAMENT_SCALE
  #include <HX711.h>
  #define EMPTY_SPOOL_WEIGHT 241
  #define ROLLER_WEIGHT     46
  #define LOADCELL_DOUT_PIN TX_PIN
  #define LOADCELL_SCK_PIN RX_PIN
  HX711 scale; // create hx711 object
  Task checkWeight = Task(0, 2500, &showWeight);

  void tareScale(){
    scale.tare();
  }
  void saveConfig() {
    StaticJsonDocument<64> doc;
    doc["calibration"] = scale.get_scale();
    doc["offset"] = scale.get_offset();
    File configFile = LittleFS.open("/config.json", "w");
    serializeJson(doc, configFile);
    configFile.close();
  }

  void countdown(int millis) {
    while (millis) {
      lcd.setCursor(15, 1);
      lcd.rightToLeft();
      lcd.print(millis / 1000);
      delay(1000);
      millis = millis - 1000;
    }
    lcd.leftToRight();
  }

  void calibrateScale() {
    lcd.clear();
    lcd.print("Calbrating");
    lcd.setCursor(0, 1);
    lcd.print("Scale");
    countdown(2000);
    scale.set_scale();
    scale.tare();
    lcd.clear();
    lcd.print("Place 100g");
    lcd.setCursor(0, 1);
    lcd.print("On Scale");
    countdown(10000);
    float raw_weight = scale.get_units(2);
    lcd.clear();
    lcd.print("raw weight");
    lcd.setCursor(0, 1);
    lcd.printf("%f", raw_weight);
    countdown(2000);
    float cal_weight = raw_weight / 100;
    lcd.clear();
    lcd.print("cal weight");
    lcd.setCursor(0, 1);
    lcd.printf("%f", cal_weight);
    countdown(2000);
    scale.set_scale(cal_weight);
    saveConfig();
    lcd.clear();
    lcd.print("Calibration");
    lcd.setCursor(0, 1);
    lcd.print("Saved");
    countdown(2000);
  }

  void loadConfig() {
    File configFile = LittleFS.open("/config.json", "r");
    if (!configFile) {
      lcd.clear();
      lcd.print("Failed to");
      lcd.setCursor(0, 1);
      lcd.print("Open Config");
      delay(4000);
      calibrateScale();
      return;
    }
    StaticJsonDocument<64> doc;
    deserializeJson(doc, configFile);
    float calValue = doc["calibration"];
    float offset = doc["offset"];
    if (offset) {
      scale.set_offset(offset);
    }
    scale.set_scale(calValue);
    delay(4000);
    lcd.clear();
    lcd.printf("calvalue%f", calValue);

    lcd.clear();
    lcd.print("Loaded");
    lcd.setCursor(0, 1);
    lcd.print("Calibration");
    configFile.close();
  }

  void showWeight() {
    long filament_remaining = 0; // hx711 returns long

    if (scale.is_ready()) {
      filament_remaining = ((scale.get_units(2)) - EMPTY_SPOOL_WEIGHT) - ROLLER_WEIGHT;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Filament Weight");
      lcd.setCursor(0, 1);
      lcd.printf("%ld grams", filament_remaining);
    }
  }
#endif

#ifdef SERVER
  AsyncWebServer server(80);
  AsyncEventSource events("/events");

  Task sendEvents(0,5000,&updateWeb);

  void startServer(){
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
      });
    server.serveStatic("/", LittleFS, "/");
    server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<64> doc;
        doc["offset"] = scale.get_offset();
        doc["reading"] = scale.get_units(2);
        String json;
        serializeJson(doc,json);
        request->send(200, "application/json", json);
        json = String();
    });
    events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){}
    client->send("hello!", NULL, millis(), 10000);
    });
    server.addHandler(&events);
    server.on("/tare", HTTP_GET, [](AsyncWebServerRequest *request){
      tareScale();
      request->send(200, "text/text", "scale tared");
    });
      server.on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *request){
      calibrateScale();
      request->send(200, "text/text", "starting calibrate");
    });
    server.begin();
  }

  void updateWeb(){
    StaticJsonDocument<64> doc;
    doc["offset"] = scale.get_offset();
    doc["reading"] = scale.get_units(2);
    String json;
    serializeJson(doc,json);
    events.send("ping",NULL,millis());
    events.send(json.c_str(),"new_readings",millis());
    json = String();
  }
#endif

void startOTA() {
  lcd.clear();
  ArduinoOTA.onStart([]() { lcd.print("Start"); });
  ArduinoOTA.onEnd([]() {
    lcd.clear();
    lcd.print("Success");
    });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    lcd.clear();
    lcd.printf("Progress: %u%%", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    lcd.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void setup() {
  LittleFS.begin();
  Wire.begin(SDA, SCL); // start i2c interface
  lcd.init();
  lcd.clear();
  lcd.backlight();

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    lcd.clear();
    lcd.print("WiFi Failed");
  }
  lcd.print(WiFi.localIP());

  startOTA();

  #ifdef SERVER
    startServer();
  #endif

  #ifdef FILAMENT_SCALE
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    loadConfig();
  #endif
}

void loop() {
  ArduinoOTA.handle();

#ifdef SERVER
  if (sendEvents.isReady()){
    sendEvents.run();
  }
#endif
#ifdef FILAMENT_SCALE
    if (checkWeight.isReady()) {
      checkWeight.run();
    }
#endif

  yield();
}