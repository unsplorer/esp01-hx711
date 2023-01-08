#include "LittleFS.h"
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <HX711.h>
#include <Adafruit_SSD1306.h>


// #define spool_weight 241
#define ROLLER_WEIGHT 46
// spool weight with rollers = 289g

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



// prototypes
void loadConfig();
void saveConfig();
void updateScale();
void calibrateScale();
void countDown(int millis);
void showWeight();
void startOTA();
void tareScale();
void startServer();
void setup();
void updateWeb();
void updateDisplay();
void justifyRight(const char *text);


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 lcd(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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


HX711 scale; // create hx711 object
Task updatedisplay = Task(0, 2500, &updateDisplay);
Task getScaleData = Task(0, 2500, &updateScale);
typedef struct scaleData {
  float weight = 0;
  float calibration = 0;
  float offset = 0;
  float spool_weight = 0;
  float filament_remaining = 0;
  bool calFlag = false;
  bool saveFlag = false;
  int knownWeight = 0;
} scaleData_t;
scaleData_t scale_data;

void updateScale() {
  if (scale.is_ready()) {
    scale_data.weight = (scale.get_units(2));
    scale_data.filament_remaining = scale_data.weight - scale_data.spool_weight;
    // if (scale_data.filament_remaining < 0) {
    //   scale_data.filament_remaining = 0;
    // }
    scale_data.calibration = scale.get_scale();
    scale_data.offset = scale.get_offset();
  }
}

void saveConfig() {
  StaticJsonDocument<64> doc;
  doc["calibration"] = scale.get_scale();
  doc["offset"] = scale.get_offset();
  doc["spool_weight"] = scale_data.spool_weight;
  File configFile = LittleFS.open("/config.json", "w");
  serializeJson(doc, configFile);
  configFile.close();
  resetDisplay();
  lcd.println("Config");
  lcd.println("saved");
  lcd.display();
  countDown(2000);
  resetDisplay();
  lcd.println("Resetting");
  lcd.println("Device");
  lcd.display();
  countDown(2000);
  ESP.reset();
}

void countDown(int millis) {
  int secondsLeft = millis / 1000;
  while (secondsLeft) {
    if (secondsLeft < 10) {
      
      lcd.setCursor(96, 48);
      lcd.fillRect(96,48,SCREEN_WIDTH-96,SCREEN_HEIGHT-48,BLACK);
      lcd.display();
      lcd.setCursor(108, 48);
    } else {
      lcd.fillRect(96, 48, SCREEN_WIDTH - 96, SCREEN_HEIGHT - 48, BLACK);
      lcd.display();
      lcd.setCursor(96, 48);
    }
    lcd.print(secondsLeft);

    lcd.display();
    delay(1000);
    secondsLeft--;
  }
  lcd.setCursor(96, 48);
  lcd.print("    ");
  lcd.display();
}

void calibrateScale() {
  if (scale_data.knownWeight == 0) {
    resetDisplay();
    lcd.println("Cal Failed");
    lcd.println("Weight\nUnKnown\n");
    lcd.display();
    scale_data.calFlag = false;
    countDown(4000);
    // ESP.reset();
    return;
  }
  resetDisplay();
  lcd.print("Calibrating");
  lcd.display();
  countDown(2000);
  scale.set_scale();
  scale.tare();
  resetDisplay();
  lcd.printf("Place %dg\nOn Scale", scale_data.knownWeight);
  lcd.display();
  countDown(5000);
  scale_data.calibration = scale.get_units(2) / scale_data.knownWeight;
  resetDisplay();
  lcd.println("cal weight");
  lcd.printf("%f\n", scale_data.calibration);
  lcd.display();
  countDown(2000);
  scale.set_scale(scale_data.calibration);
  scale_data.saveFlag = true;
  scale_data.calFlag = false;
}

void loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    resetDisplay();
    lcd.println("Failed");
    lcd.println("to load");
    lcd.println("config");
    lcd.display();
    delay(4000);
    return;
  }
  StaticJsonDocument<96> doc;
  deserializeJson(doc, configFile);
  scale_data.calibration = doc["calibration"];
  scale_data.offset = doc["offset"];
  scale_data.spool_weight = doc["spool_weight"];
  if (scale_data.offset) {
    scale.set_offset(scale_data.offset);
  }
  if (scale_data.calibration) {
    scale.set_scale(scale_data.calibration);
  }
  resetDisplay();
  lcd.println("Loaded");
  lcd.println("config");
  lcd.display();
  countDown(2000);
  configFile.close();
}

/**********************************************************************/
/*!
  @brief  Output right justified text
  @param text input text
*/
/**********************************************************************/
void justifyRight(const char* text) {
  int16_t x1, y1;
  uint16_t w, h;

  lcd.getTextBounds(text, lcd.getCursorX(), lcd.getCursorY(), &x1, &y1, &w, &h);
  lcd.setCursor((SCREEN_WIDTH - w), lcd.getCursorY());
  lcd.println(text);
}
/**********************************************************************/
/*!
  @brief  Output centered text
  @param text input text
*/
/**********************************************************************/
void centerText(const char* text) {
  int16_t x1, y1;
  uint16_t w, h;

  lcd.getTextBounds(text, lcd.getCursorX(), lcd.getCursorY(), &x1, &y1, &w, &h);
  lcd.setCursor((SCREEN_WIDTH - w) / 2, lcd.getCursorY());
  lcd.println(text);
}
/**********************************************************************/
/*!
  @brief  Update weight on Display
*/
/**********************************************************************/
void showWeight() {
  
  char filamentRemaining[8];
  sprintf(filamentRemaining, "%.0fg\n",scale_data.filament_remaining);

  resetDisplay();
  lcd.setTextSize(1);
  centerText("Filament Remaining\n");
  lcd.setTextSize(3);
  centerText(filamentRemaining);
  lcd.setTextSize(2);
}

/**********************************************************************/
/*!
  @brief  Update WiFi status on Display
*/
/**********************************************************************/
void showWiFiStatus(){
  char ip[24] = {};
  sprintf(ip,"%s",WiFi.localIP().toString().c_str());
  lcd.setTextSize(1);
  lcd.setCursor(0, 48);
  lcd.print("Hostname:");
  justifyRight(WiFi.hostname().c_str());
  lcd.setCursor(0, 56);
  lcd.print("IP:");
  justifyRight(ip);
  lcd.setTextSize(2);
}

/**********************************************************************/
/*!
  @brief  Update display
*/
/**********************************************************************/
void updateDisplay() {
  resetDisplay();
  showWeight();
  showWiFiStatus();
  lcd.display();
}

/**********************************************************************/
/*!
  SERVER SECTION
*/
/**********************************************************************/
AsyncWebServer server(80);
AsyncEventSource events("/events");

Task sendEvents(0, 1250, &updateWeb);

void startServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<64> doc;
    doc["reading"] = scale_data.filament_remaining;
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
    json = String();
  });

  server.on("/tare", HTTP_POST, [](AsyncWebServerRequest *request) {
    scale.tare(2);
    scale_data.saveFlag = true;
    request->send(200);
  });

  server.on("/calibrate", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("known_weight", true)) {
      scale_data.calFlag = true;
      const char *weight = request->getParam("known_weight", true)->value().c_str();
      scale_data.knownWeight = atoi(weight);
      request->redirect("/");
      request->send(200);
    } else {
      request->send(500);
    }
  });

  server.on("/spool", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("spool", true)) {
      const char *value = request->getParam("spool", true)->value().c_str();
      scale_data.spool_weight = atoi(value);
      scale_data.saveFlag = true;
      request->redirect("/");
      request->send(200);
    } else {
      request->send(500);
    }
  });

  server.addHandler(&events);
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
    }
    client->send("hello!", NULL, millis(), 10000);
  });

  server.begin();
}


double round2(double value) {
   return (int)(value * 100 + 0.5) / 100.0;
}


void updateWeb() {
  StaticJsonDocument<96> doc;
  doc["reading"] = round2(scale_data.filament_remaining);
  doc["raw_weight"] = round2(scale_data.weight);
  doc["spool_weight"] = round2(scale_data.spool_weight);
  doc["calibration_value"] = round2(scale_data.calibration);
  doc["offset"] = scale_data.offset;
  String json;
  serializeJson(doc, json);
  events.send(json.c_str(), "new_readings", millis());
}
/**********************************************************************/
/*!
  END SERVER SECTION
*/
/**********************************************************************/

void startOTA() {
  // resetDisplay();
  ArduinoOTA.onStart([]() {
    // events.close();
    // server.end();
    resetDisplay();
    lcd.println("Start");
    lcd.display();
  });
  ArduinoOTA.onEnd([]() {
    resetDisplay();
    lcd.println("Success");
    lcd.display();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // resetDisplay();
    // char progressPercent[16];
    // sprintf(progressPercent,"%u%%",(progress / (total / 100)));
    // lcd.println("Updating");
    // lcd.setCursor(0, 16);
    // lcd.print("Progress:");
    // lcd.setCursor(0,48);
    // justifyRight(progressPercent);
    // lcd.display();
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

  lcd.printf("Trying\nSSID:\n%s", WiFi.SSID().c_str());
  lcd.display();
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (retry++ >= 40) {
      resetDisplay();
      lcd.printf("WiFi\ntimeout\n");
      lcd.display();
      delay(250);
      WiFi.beginSmartConfig();
      // forever loop: exit only when SmartConfig packets have been received
      while (true) {
        delay(500);
        resetDisplay();
        lcd.setTextSize(1);
        lcd.println("Waiting...\n\n");
        lcd.println("Use Smartconfig");
        lcd.println("app to setup");
        lcd.println("WiFi");
        lcd.display();
        lcd.setTextSize(2);
        if (WiFi.smartConfigDone()) {
          resetDisplay();
          lcd.printf("WiFi\nConfig\nSuccess");
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
}

void setup() {
  LittleFS.begin();
  Wire.begin(SDA, SCL); // start i2c interface
  initDisplay();
  setupWiFi();
  startOTA();
  startServer();
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadConfig();
}

void loop() {
  ArduinoOTA.handle();

  if (sendEvents.isReady()) {
    sendEvents.run();
  }

  if(scale_data.saveFlag){
    server.end();
    saveConfig();
  }

  if (scale_data.calFlag) {
    server.end();
    calibrateScale();
  }

  if (getScaleData.isReady()) {
    getScaleData.run();
  }

  if (updatedisplay.isReady()) {
    updatedisplay.run();
  }

  yield();
}