#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <LiquidCrystal_I2C.h>
#include <LittleFS.h>
#include <WiFiUdp.h>
#include <Wire.h>
// #include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define SSID "RodHodder"
#define PASS "!q2w3e4ribanezS450mitantway85"

#define FILAMENT_SCALE // enable FILAMENT_SCALE component
#define CAL_WEIGHT 311.f
#define SDA 0x00
#define SCL 0x02
#define RX_PIN 0x03
#define TX_PIN 0x01

ESP8266WebServer server(80);
// WiFiServer server(80);
LiquidCrystal_I2C
    lcd(0x27, 16,
        2); // Create lcd object at address 0x27 with 16 columns , 2 rows

#ifdef FILAMENT_SCALE
#include <HX711.h>
#define EMPTY_SPOOL_WEIGHT 235
#define LOADCELL_DOUT_PIN TX_PIN
#define LOADCELL_SCK_PIN RX_PIN
HX711 scale; // create hx711 object

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

void showError(const char *error) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error :");
  lcd.setCursor(0, 1);
  lcd.printf("%s", error);
}

void showWeight() {
  long filament_remaining = 0; // hx711 returns long

  if (scale.is_ready()) {
    filament_remaining = (scale.get_units(2)) - EMPTY_SPOOL_WEIGHT;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Filament Weight");
    lcd.setCursor(0, 1);
    lcd.printf("%ld grams", filament_remaining);
    if (filament_remaining > 2000 || filament_remaining < -275) {
      calibrateScale();
    }
  } else {
    showError("Weigh Failed"); // scale failed, display error
  }
}
#endif

// class Test
// {
// private:
//   unsigned long lastRun;
//   unsigned int interval;
//   void (*task)();

// public:
//   Test(unsigned long lastRun, unsigned int interval,void (func)()){
//     this->task = func;
//     this->lastRun = lastRun;
//     this->interval = interval;
//   };
//   void run(){
//     this->task();
//   }
//   bool isReady(){
//     return ((millis() - this->lastRun) > this->interval) ? true : false;
//   }
// };

// Test updateScale{0,2500,showWeight};

class Task {
private:
  unsigned long lastRun;
  unsigned int interval;
  std::function<void()> actualTask_;

public:
  Task(unsigned long lastRun, unsigned int interval,
       const std::function<void()> &functionToRun)
      : actualTask_(functionToRun) {
    this->lastRun = lastRun;
    this->interval = interval;
  };

  void setTask(const std::function<void()> &functionToRun) {
    actualTask_ = functionToRun;
  }

  void run() {
    this->lastRun = millis();
    return actualTask_();
  }

  bool isReady() {
    return ((millis() - this->lastRun) > this->interval) ? true : false;
  }
};

Task checkWeight = Task(0, 2500, showWeight);

void startOTA() {
  lcd.clear();
  ArduinoOTA.onStart([]() { lcd.print("Start"); });
  ArduinoOTA.onEnd([]() { lcd.print("End"); });
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

void handle_home_page() {
  float weight = scale.get_units(2);
  char body[1024];
  sprintf(body,
          "<html> <head>   <title>ESP8266 Page</title> <meta name='viewport' "
          "content='width=device-width, initial-scale=1.0'>  <style>     h1 "
          "{text-align:center; }     td {font-size: 50%; padding-top: 30px;}   "
          "  .temp {font-size:150%; color: #FF0000;}     .press "
          "{font-size:150%; color: #00FF00;}     .hum {font-size:150%; color: "
          "#0000FF;}   </style> </head>  <body>    <h1>ESP8266 Sensor "
          "Page</h1>    <div id='div1'>        <table>           <tr>          "
          "  <td>Offset</td><td class='temp'>%ld</td>          </tr>     "
          "     <tr>   <td>Weight</td><td class='press'>%f</td>   </tr>  "
          "<tr>   </tr> </div> "
          "</body>  </html>",
          scale.get_offset(),weight);
  server.send(200, "text/html", body);
}

void tareScale(){
  scale.tare();
}
void setup() {
  LittleFS.begin();
  Wire.begin(SDA, SCL); // start i2c interface
  lcd.init();

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    lcd.clear();
    lcd.print("WiFi Failed");
  }
  startOTA();
  server.on("/", handle_home_page);
  server.on("/tare", tareScale);
  server.on("/calibrate", calibrateScale);
  server.begin();

  lcd.clear();
  lcd.backlight();
  lcd.print(WiFi.localIP());

#ifdef FILAMENT_SCALE
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadConfig();
#endif
}

void handleHTTP(WiFiClient &client) {
  static unsigned long previousTime = millis();
  const int timeoutTime = 5000;
  String header = "";
  String currentLine = "";
  while (client.connected() && millis() - previousTime <= timeoutTime) {
    char c = client.read();
    header += c;
    if (c == '\n') { // if the byte is a newline character
      // if the current line is blank, you got two newline characters in a row.
      // that's the end of the client HTTP request, so send a response:
      if (currentLine.length() == 0) {
        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
        // and a content-type so the client knows what's coming, then a blank
        // line:
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println();

        // turns the GPIOs on and off
        if (header.indexOf("GET /5/on") >= 0) {
        }

        // Display the HTML web page
        client.println("<!DOCTYPE html><html>");
        client.println("<head><meta name=\"viewport\" "
                       "content=\"width=device-width, initial-scale=1\">");
        client.println("<link rel=\"icon\" href=\"data:,\">");
        // CSS to style the on/off buttons
        // Feel free to change the background-color and font-size attributes to
        // fit your preferences
        client.println("<style>html { font-family: Helvetica; display: "
                       "inline-block; margin: 0px auto; text-align: center;}");
        client.println(".button { background-color: #195B6A; border: none; "
                       "color: white; padding: 16px 40px;");
        client.println("text-decoration: none; font-size: 30px; margin: 2px; "
                       "cursor: pointer;}");
        client.println(".button2 {background-color: #77878A;}</style></head>");

        // Web Page Heading
        client.println("<body><h1>ESP8266 Web Server</h1>");

        client.println("</body></html>");

        // The HTTP response ends with another blank line
        client.println();
        // Break out of the while loop
        break;
      } else { // if you got a newline, then clear currentLine
        currentLine = "";
      }
    } else if (c != '\r') { // if you got anything else but a carriage return
                            // character,
      currentLine += c;     // add it to the end of the currentLine
    }
  }
}


void loop() {
  // WiFiClient client = server.available();         // listen for clients
  // if (client) {
  //   lcd.clear();
  //   lcd.print("client conn");
  //   handleHTTP(client);
  //   client.stop();
  // }
  server.handleClient();

  ArduinoOTA.handle();

#ifdef FILAMENT_SCALE
  if (checkWeight.isReady()) {
    checkWeight.run();
  }
#endif
  yield();
}