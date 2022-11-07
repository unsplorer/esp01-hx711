#include "secrets.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_I2C.h>
#include <LittleFS.h>
#include <Wire.h>

#define FILAMENT_SCALE // enable FILAMENT_SCALE component
#define SERVER
#define SDA 0x00
#define SCL 0x02
#define RX_PIN 0x03
#define TX_PIN 0x01
// prototypes
void loadConfig();
void saveConfig();
void updateScale();
void calibrateScale();
void countdown(int millis);
void showWeight();
void startOTA();
void tareScale();
void startServer();
void setup();
void updateWeb();

LiquidCrystal_I2C lcd(0x27, 16, 2);

class Task {
private:
  unsigned long lastRun;
  unsigned int interval;
  void (*_actualTask_)();

public:
  Task(unsigned long lastRun, unsigned int interval, void(func)()) {
    this->lastRun = lastRun;
    this->interval = interval;
    this->_actualTask_ = func;
  }
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
#define ROLLER_WEIGHT 46
#define LOADCELL_DOUT_PIN TX_PIN
#define LOADCELL_SCK_PIN RX_PIN
HX711 scale; // create hx711 object
Task updateDisplayWeight = Task(0, 2500, &showWeight);
Task getScaleData = Task(0, 2500, &updateScale);
typedef struct scaleData {
  float weight = 0;
  float calibration= 0;
  float offset = 0;
  float filament_remaining = 0;
  bool calFlag = false;
  int knownWeight = 0;
} scaleData_t;
scaleData_t scale_data;

void updateScale() {
  if (scale.is_ready()) {
    scale_data.weight = (scale.get_units(2));
    scale_data.filament_remaining = scale_data.weight - EMPTY_SPOOL_WEIGHT;
    if (scale_data.filament_remaining < 0) {
      scale_data.filament_remaining = 0;
    }
    scale_data.calibration = scale.get_scale();
    scale_data.offset = scale.get_offset();
  }
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
  if (scale_data.knownWeight == 0){
    lcd.clear();
    lcd.print("Cal Failed");
    lcd.setCursor(0, 1);
    lcd.print("Weight UnKnown");
    scale_data.calFlag = false;
    delay(4000);
    // ESP.reset();
    return;
  }
  lcd.clear();
  lcd.print("Calbrating");
  lcd.setCursor(0, 1);
  lcd.print("Scale");
  countdown(2000);
  scale.set_scale();
  scale.tare();
  lcd.clear();
  lcd.printf("Place %dg",scale_data.knownWeight);
  lcd.setCursor(0, 1);
  lcd.print("On Scale");
  countdown(5000);
  scale_data.calibration = scale.get_units(2) / scale_data.knownWeight;
  lcd.clear();
  lcd.print("cal weight");
  lcd.setCursor(0, 1);
  lcd.printf("%f", scale_data.calibration);
  countdown(2000);
  scale.set_scale(scale_data.calibration);
  saveConfig();
  lcd.clear();
  lcd.print("Calibration");
  lcd.setCursor(0, 1);
  lcd.print("Saved");
  countdown(2000);
  scale_data.calFlag = false;
  // ESP.reset();
}

void loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    lcd.clear();
    lcd.print("Failed to Load");
    lcd.setCursor(0, 1);
    lcd.print("Calibration");
    delay(4000);
    return;
  }
  StaticJsonDocument<64> doc;
  deserializeJson(doc, configFile);
  scale_data.calibration = doc["calibration"];
  scale_data.offset= doc["offset"];
  if (scale_data.offset) {
    scale.set_offset(scale_data.offset);
  }
  if (scale_data.calibration){
    scale.set_scale(scale_data.calibration);
  }
  lcd.clear();
  lcd.print("Loaded");
  lcd.setCursor(0, 1);
  lcd.print("Calibration");
  lcd.clear();
  lcd.printf("calvalue%.2f", scale_data.calibration);
  countdown(4000);
  configFile.close();
}

void showWeight() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Filament Weight");
  lcd.setCursor(0, 1);
  lcd.printf("%.2f grams", scale_data.filament_remaining);
}
#endif

#ifdef SERVER
AsyncWebServer server(80);
AsyncEventSource events("/events");

Task sendEvents(0, 5000, &updateWeb);

void startServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<64> doc;
    doc["reading"] = scale_data.filament_remaining;
    doc["offset"] = scale_data.offset;
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
    json = String();
  });
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.on("/tare", HTTP_GET, [](AsyncWebServerRequest *request) {
    scale.tare(2);
  });
  server.on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *request) {
    scale_data.calFlag = true;
    String message;
    if (request->hasParam("known_weight", false)){
      message = request->getParam("known_weight")->value();
    }
    scale_data.knownWeight = atoi(message.c_str());
  });
  server.on("/offset", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam("offset", false)){
      message = request->getParam("offset")->value();
    }
    scale_data.offset = atoi(message.c_str());
    scale.set_offset(scale_data.offset);
  });
  server.begin();
}

void updateWeb() {
  StaticJsonDocument<64> doc;
  doc["reading"] = scale_data.filament_remaining;
  String json;
  serializeJson(doc, json);
  events.send("ping", NULL, millis());
  events.send(json.c_str(), "new_readings", millis());
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
    lcd.print("Updating");
    lcd.setCursor(0,1);
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
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
  if (sendEvents.isReady()) {
    sendEvents.run();
  }
#endif

#ifdef FILAMENT_SCALE
  if (scale_data.calFlag){
    server.end();
    calibrateScale();
    startServer();
  }
  if (getScaleData.isReady()) {
    getScaleData.run();
  }
  if (updateDisplayWeight.isReady()) {
    updateDisplayWeight.run();
  }
#endif

  yield();
}