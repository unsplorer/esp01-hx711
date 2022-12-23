#include "LittleFS.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

// #define OTA        //  enable OTA updating
#define SSD1306
// #define LCD1602
#define FILAMENT_SCALE // enable FILAMENT_SCALE component
#define SERVER         // enable webserver
// ESP01 build pinouts
#ifdef ESP01
#define SDA 0x02
#define SCL 0x00
#define RX_PIN 0x03
#define TX_PIN 0x01
#define LOADCELL_DOUT_PIN TX_PIN
#define LOADCELL_SCK_PIN RX_PIN
#endif
// nodemcu build pinouts
#ifdef NODEMCUV2
#define LOADCELL_DOUT_PIN D5
#define LOADCELL_SCK_PIN D6
#endif

#ifdef OTA
#include <ArduinoOTA.h>
#endif

#ifdef SERVER
#include <ESPAsyncWebServer.h>
#endif

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

#ifdef LCD1602
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif
#ifdef SSD1306
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 lcd(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif
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

void resetDisplay() {
  lcd.clearDisplay();
  lcd.setCursor(0, 0);
}

#ifdef FILAMENT_SCALE
#include <HX711.h>
#define EMPTY_SPOOL_WEIGHT 241
#define ROLLER_WEIGHT 46
HX711 scale; // create hx711 object
Task updateDisplayWeight = Task(0, 2500, &showWeight);
Task getScaleData = Task(0, 2500, &updateScale);
typedef struct scaleData {
  float weight = 0;
  float calibration = 0;
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
    lcd.setCursor(116, 63);
    lcd.print(millis / 1000);
    lcd.display();
    delay(1000);
    millis = millis - 1000;
  }
}

void calibrateScale() {
  if (scale_data.knownWeight == 0) {
    resetDisplay();
    lcd.println("Cal Failed");
    lcd.println("Weight UnKnown");
    lcd.display();
    scale_data.calFlag = false;
    delay(4000);
    // ESP.reset();
    return;
  }
  resetDisplay();
  lcd.print("Calbrating");
  lcd.setCursor(0, 1);
  lcd.print("Scale");
  lcd.display();
  countdown(2000);
  scale.set_scale();
  scale.tare();
  resetDisplay();
  lcd.printf("Place %dg", scale_data.knownWeight);
  lcd.setCursor(0, 1);
  lcd.print("On Scale");
  lcd.display();
  countdown(5000);
  scale_data.calibration = scale.get_units(2) / scale_data.knownWeight;
  resetDisplay();
  lcd.print("cal weight");
  lcd.setCursor(0, 1);
  lcd.printf("%f", scale_data.calibration);
  lcd.display();
  countdown(2000);
  scale.set_scale(scale_data.calibration);
  saveConfig();
  resetDisplay();
  lcd.print("Calibration");
  lcd.setCursor(0, 1);
  lcd.print("Saved");
  lcd.display();
  countdown(2000);
  resetDisplay();
  lcd.print("Resetting");
  lcd.setCursor(0, 1);
  lcd.print("Device");
  scale_data.calFlag = false;
  ESP.reset();
}

void loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    resetDisplay();
    lcd.print("Failed to Load");
    lcd.setCursor(0, 1);
    lcd.print("Calibration");
    lcd.display();
    delay(4000);
    return;
  }
  StaticJsonDocument<64> doc;
  deserializeJson(doc, configFile);
  scale_data.calibration = doc["calibration"];
  scale_data.offset = doc["offset"];
  if (scale_data.offset) {
    scale.set_offset(scale_data.offset);
  }
  if (scale_data.calibration) {
    scale.set_scale(scale_data.calibration);
  }
  resetDisplay();
  lcd.println("Loaded");
  lcd.println("Calibration");
  lcd.printf("calvalue%.2f", scale_data.calibration);
  lcd.display();
  countdown(4000);
  configFile.close();
}

void showWeight() {
  resetDisplay();
  lcd.setCursor(0, 0);
  lcd.println("Filament");
  lcd.printf("%.2f g\n", scale_data.filament_remaining);
  lcd.display();
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
  server.on("/tare", HTTP_GET,
            [](AsyncWebServerRequest *request) { scale.tare(2); });
  server.on("/calibrate", HTTP_POST, [](AsyncWebServerRequest *request) {
    scale_data.calFlag = true;
    String message;
    if (request->hasParam("known_weight", true)) {
      message = request->getParam("known_weight")->value();
    }
    scale_data.knownWeight = atoi(message.c_str());
  });
  server.on("/offset", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam("offset", false)) {
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

#ifdef OTA
void startOTA() {
  resetDisplay();
  ArduinoOTA.onStart([]() {
    lcd.println("Start");
    lcd.display();
  });
  ArduinoOTA.onEnd([]() {
    resetDisplay();
    lcd.println("Success");
    lcd.display();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    resetDisplay();
    lcd.println("Updating");
    lcd.setCursor(0, 16);
    lcd.printf("Progress: %u%%", (progress / (total / 100)));
    lcd.display();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    lcd.printf("Error[%u]: ", error);
    lcd.display();
    if (error == OTA_AUTH_ERROR)
      lcd.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      lcd.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      lcd.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      lcd.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      lcd.println("End Failed");
  });
  ArduinoOTA.begin();
}
#endif

void initDisplay() {
  if (lcd.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    resetDisplay();
    lcd.setTextSize(2);
    lcd.setTextColor(SSD1306_WHITE);
    lcd.cp437(true);

    // Display test
    lcd.invertDisplay(true);
    lcd.display();
    delay(1000);
    lcd.invertDisplay(false);
    lcd.display();
    delay(1000);
  }
}

void setupWiFi() {
  int retry = 0, config_done = 0;
  WiFi.mode(WIFI_STA);

  // check whether WiFi connection can be established
  resetDisplay();

  lcd.printf("Connecting to \n SSID: %s", WiFi.SSID().c_str());
  lcd.display();
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (retry++ >= 40) {
      resetDisplay();
      lcd.println("Connection timeout");
      lcd.display();
      delay(250);
      WiFi.beginSmartConfig();
      // forever loop: exit only when SmartConfig packets have been received
      while (true) {
        delay(500);
        resetDisplay();
        lcd.println("Waiting...\n\n");
        lcd.println("Use Smartconfig");
        lcd.println("app to setup");
        lcd.println("WiFi");
        lcd.display();
        if (WiFi.smartConfigDone()) {
          resetDisplay();
          lcd.println("WiFi Config Success");
          lcd.display();
          config_done = 1;
          break; // exit from loop
        }
      }
      if (config_done == 1) {
        WiFi.persistent(true);
        WiFi.setAutoReconnect(true);
        break;
      }
    }
  }
  resetDisplay();
  lcd.printf("SSID: %s\nIP:", WiFi.SSID().c_str());
  lcd.println(WiFi.localIP());
  lcd.printf("Hostname: %s\n", WiFi.hostname().c_str());
  lcd.display();
}

void setup() {
  // Serial.begin(9600);
  LittleFS.begin();
  Wire.begin(SDA, SCL); // start i2c interface
  initDisplay();
  setupWiFi();

#ifdef OTA
  startOTA();
#endif

#ifdef SERVER
  startServer();
#endif

#ifdef FILAMENT_SCALE
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadConfig();
#endif
}

void loop() {
#ifdef OTA
  ArduinoOTA.handle();
#endif

#ifdef SERVER
  if (sendEvents.isReady()) {
    sendEvents.run();
  }
#endif

#ifdef FILAMENT_SCALE
  if (scale_data.calFlag) {
#ifdef SERVER
    server.end();
#endif
    calibrateScale();
#ifdef SERVER
    startServer();
#endif
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