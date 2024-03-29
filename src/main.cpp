#include "LittleFS.h"
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <HX711.h>
#include <Adafruit_SSD1306.h>
#include "task.h"
// #define spool_weight 241
// #define ROLLER_WEIGHT 46
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

void resetDisplay();
void loadConfig();
void saveConfig();
void updateScale();
void calibrateScale(int calibrationWeight);
void countDown(int millis);
void showWeight();
void startOTA();
void startServer();
void setup();
void updateWeb();
void updateDisplay();
void justifyRight(const char *text);
double round2(double value);

// Used to hold all data pertaining to scale
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

/**********************************************************************/
/*!
    DISPLAY SECTION START
*/
/**********************************************************************/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 lcd(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Task updatedisplay = Task(0, 2500, &updateDisplay);


// 'splash_logo', 128x64px
const unsigned char splash_logo [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0xe0, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0f, 0x06, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x1c, 0x0f, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x38, 0x1f, 0x81, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x70, 0x1f, 0x80, 0xe0, 0x00, 0x00, 0x07, 0x80, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xe0, 0x1f, 0x80, 0x70, 0x00, 0x00, 0x0f, 0xcc, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xc0, 0x1f, 0x80, 0x30, 0x00, 0x00, 0x18, 0x7e, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0x80, 0x1f, 0x80, 0x18, 0x00, 0x00, 0x18, 0x2c, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0x80, 0x1f, 0x80, 0x18, 0x00, 0x00, 0x18, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x80, 0x0f, 0x00, 0x1c, 0x00, 0x00, 0x18, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x00, 0x0f, 0x00, 0x0c, 0x00, 0x00, 0xff, 0x0c, 0x60, 0x7e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x18, 0x0c, 0x60, 0xc3, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x00, 0x0e, 0x00, 0x0c, 0x00, 0x00, 0x18, 0x0c, 0x61, 0x81, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x00, 0x0f, 0x00, 0x0c, 0x00, 0x00, 0x18, 0x0c, 0x63, 0x01, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x07, 0x00, 0x1f, 0x80, 0x0c, 0x00, 0x00, 0x18, 0x0c, 0x63, 0x01, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x00, 0x0f, 0x00, 0x0c, 0x00, 0x00, 0x18, 0x0c, 0x62, 0x03, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x00, 0xe6, 0x70, 0x0c, 0x00, 0x00, 0x18, 0x0c, 0x62, 0x03, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x01, 0xf0, 0xf8, 0x0c, 0x00, 0x00, 0x18, 0x0c, 0x62, 0x07, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x03, 0xb0, 0xdc, 0x0c, 0x00, 0x00, 0x18, 0x0c, 0x62, 0x0f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x87, 0x30, 0xce, 0x1c, 0x00, 0x00, 0x18, 0x0c, 0x63, 0x1d, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0x8e, 0x70, 0xe7, 0x18, 0x00, 0x00, 0x18, 0x08, 0x61, 0xf9, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0x8c, 0xe0, 0x73, 0x38, 0x00, 0x00, 0x18, 0x08, 0x60, 0xe1, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xcd, 0xc0, 0x3b, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0xf6, 0x00, 0x06, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x9c, 0xff, 0xf3, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xcf, 0x00, 0x0f, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x63, 0xe0, 0x7c, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0xff, 0xf1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0x3f, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x18, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x08, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x08, 0x3e, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x08, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0xc0, 0x30, 0x80, 0x00, 0x00, 0x0e, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 
	0x00, 0x00, 0x11, 0x00, 0x08, 0x80, 0x00, 0x00, 0x3f, 0x90, 0x1f, 0xc6, 0x38, 0x9f, 0x80, 0x00, 
	0x00, 0x00, 0x11, 0x00, 0x08, 0x80, 0x00, 0x00, 0x30, 0xf0, 0x78, 0xc6, 0x60, 0xb0, 0xc0, 0x00, 
	0x00, 0x00, 0x22, 0x00, 0x04, 0x40, 0x00, 0x00, 0x60, 0x70, 0x60, 0x62, 0xc0, 0xe0, 0x60, 0x00, 
	0x00, 0x00, 0x22, 0x06, 0x04, 0x40, 0x00, 0x00, 0x40, 0x30, 0xc0, 0x63, 0x80, 0xc0, 0x20, 0x00, 
	0x00, 0x00, 0x23, 0x07, 0xcc, 0x40, 0x00, 0x00, 0xc0, 0x11, 0x80, 0xc3, 0x80, 0xc0, 0x30, 0x00, 
	0x00, 0x00, 0x62, 0x06, 0x04, 0x20, 0x00, 0x00, 0xc0, 0x11, 0x83, 0x83, 0x00, 0xc0, 0x30, 0x00, 
	0x00, 0x00, 0x42, 0x00, 0x00, 0x20, 0x00, 0x00, 0xc0, 0x31, 0xff, 0x03, 0x00, 0x80, 0x30, 0x00, 
	0x00, 0x00, 0x41, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x31, 0xf8, 0x03, 0x00, 0xc0, 0x30, 0x00, 
	0x00, 0x00, 0x41, 0x40, 0x28, 0x30, 0x00, 0x00, 0x60, 0x71, 0x80, 0x02, 0x00, 0xe0, 0x20, 0x00, 
	0x00, 0x00, 0x80, 0xc0, 0x30, 0x10, 0x00, 0x00, 0x70, 0xf0, 0xc0, 0x66, 0x00, 0xf0, 0x60, 0x00, 
	0x00, 0x00, 0x80, 0x60, 0x20, 0x10, 0x00, 0x00, 0x3f, 0xd0, 0x77, 0xc6, 0x00, 0xdd, 0xc0, 0x00, 
	0x00, 0x00, 0x80, 0x3f, 0xc0, 0x10, 0x00, 0x00, 0x0f, 0x10, 0x3f, 0x06, 0x00, 0xcf, 0x80, 0x00, 
	0x00, 0x00, 0x40, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/**********************************************************************/
/*!
  @brief  Initializes the display and shows splash screen.
*/
/**********************************************************************/
void initDisplay() {
  if (lcd.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    resetDisplay();
    lcd.setTextSize(2);
    lcd.setTextColor(SSD1306_WHITE);
    lcd.cp437(true);
    lcd.drawBitmap(0,0,splash_logo,128,64,WHITE);
    lcd.display();
    delay(2000);
  }
}


/**********************************************************************/
/*!
  @brief  Clear display and set cursor to top left.
*/
/**********************************************************************/
void resetDisplay() {
  lcd.clearDisplay();
  lcd.setCursor(0, 0);
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
  @brief  Updates the weight section on Display
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
  @brief  Updates WiFi status section on Display
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
  @brief  Main display routine, shows current filament weight,
  WiFi hostname, and IP address
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
  @brief  display countdown and wait in seconds on lower right display
  @param millis time in milliseconds to countdown
*/
/**********************************************************************/
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


/**********************************************************************/
/*!
    DISPLAY SECTION END
*/
/**********************************************************************/

/**********************************************************************/
/*!
    SCALE SECTION START
*/
/**********************************************************************/

HX711 scale; // create hx711 object
Task getScaleData = Task(0, 2500, &updateScale);


/**********************************************************************/
/*!
    @brief Loads calibration values from persistent storage. Display 
    a warning if no calibration values have been stored yet.
*/
/**********************************************************************/
void loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");

  // file doesn't exist, show warning.
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
    @brief Helper function that saves scale_data to persistent storage
    to manage data loss in between reboots / powerdown. Function is 
    triggered by saveFlag in scale_data struct. Save happens in main loop
    then triggers a device reset.
*/
/**********************************************************************/
void saveConfig() {
  StaticJsonDocument<64> doc;
  doc["calibration"] = scale_data.calibration;
  doc["offset"] = scale_data.offset;
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


/**********************************************************************/
/*!
    @brief Calibrates the scale and sets save flag
    @param calibrationWeight "known weight" passed in to calibrate the 
    scale against
*/
/**********************************************************************/
void calibrateScale(int calibrationWeight) {
  // Impossible to calibrate against negative or no weight
  // reset the device after displaying warning.
  if (calibrationWeight <= 0) {
    resetDisplay();
    lcd.println("Cal Failed");
    lcd.println("Weight\nUnKnown\n");
    lcd.display();
    scale_data.calFlag = false;
    countDown(4000);
    ESP.reset();
    return;
  }

  resetDisplay();
  lcd.print("Calibrating");
  lcd.display();
  countDown(2000);
  scale.set_scale();
  scale.tare();
  resetDisplay();
  lcd.printf("Place %dg\nOn Scale", calibrationWeight);
  lcd.display();
  countDown(5000);
  scale_data.calibration = scale.get_units(2) / calibrationWeight;
  resetDisplay();
  lcd.println("cal weight");
  lcd.printf("%f\n", scale_data.calibration);
  lcd.display();
  countDown(2000);
  scale.set_scale(scale_data.calibration);
  // save the new calibration data, then reset device
  scale_data.saveFlag = true;
  scale_data.calFlag = false;
}


/**********************************************************************/
/*!
    @brief queries the scale and gets current weight, calcs filament remaning
    from tare / empty spool weight.
*/
/**********************************************************************/
void updateScale() {
  if (scale.is_ready()) {
    scale_data.weight = (scale.get_units(2));
    scale_data.filament_remaining = scale_data.weight - scale_data.spool_weight;
    // scale_data.calibration = scale.get_scale();
    // scale_data.offset = scale.get_offset();
  }
}


/**********************************************************************/
/*!
  SERVER SECTION
*/
/**********************************************************************/
AsyncWebServer server(80);
AsyncEventSource events("/events");

Task sendEvents(0, 1250, &updateWeb);


/**********************************************************************/
/*!
    @brief Initializes the async webserver and sets up routing
*/
/**********************************************************************/
void startServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  server.on("/tare", HTTP_POST, [](AsyncWebServerRequest *request) {
    scale.tare(2);
    scale_data.offset = scale.get_offset();
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

  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<512> doc;
    JsonObject device = doc.createNestedObject("device");
    JsonObject scale = doc.createNestedObject("scale");

    device["ssid"] = WiFi.SSID();
    device["rssi"] = WiFi.RSSI();
    device["uptime"] = millis();

    scale["filament_remaining"] = round2(scale_data.filament_remaining);
    scale["spool_weight"] = round2(scale_data.spool_weight);
    scale["calibration_value"] = round2(scale_data.calibration);
    scale["offset"] = scale_data.offset;

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  server.on("/api", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("offset", true)) {
      const char *offset = request->getParam("offset", true)->value().c_str();
      if (atoi(offset)){
        scale_data.offset = atoi(offset);
        scale_data.saveFlag = true;
      }
    } 
    if (request->hasParam("calvalue", true)){
      const char *calvalue = request->getParam("calvalue", true)->value().c_str();
      if (atof(calvalue)){
        scale_data.calibration = atof(calvalue);
        scale_data.saveFlag = true;
      }
    } 
    if (request->hasParam("spool_weight", true)){
      const char *spool_weight = request->getParam("spool_weight", true)->value().c_str();
      if (atoi(spool_weight)){
        scale_data.spool_weight = atoi(spool_weight);
        scale_data.saveFlag = true;
      }
    }
    
    request->redirect("/");
    request->send(200);
  });

  server.addHandler(&events);
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
    }
    client->send("hello!", NULL, millis(), 10000);
  });

  server.begin();
}


/**********************************************************************/
/*!
    @brief Helper function to round floats down to 2 decimals
    @param value Number to round
*/
/**********************************************************************/
double round2(double value) {
   return (int)(value * 100 + 0.5) / 100.0;
}


/**********************************************************************/
/*!
    @brief Server side event backend, pushes data to the frontend webpage
*/
/**********************************************************************/
void updateWeb() {
  StaticJsonDocument<512> doc;
  JsonObject device = doc.createNestedObject("device");
  JsonObject scale = doc.createNestedObject("scale");

  device["ssid"] = WiFi.SSID();
  device["rssi"] = WiFi.RSSI();
  device["uptime"] = millis();

  scale["filament_remaining"] = round2(scale_data.filament_remaining);
  scale["spool_weight"] = round2(scale_data.spool_weight);
  scale["calibration_value"] = round2(scale_data.calibration);
  scale["offset"] = scale_data.offset;

  char buffer[512];
  serializeJson(doc,buffer);
  // String json;
  // serializeJson(doc, json);
  events.send(buffer, "report", millis());
}


/**********************************************************************/
/*!
  END SERVER SECTION
*/
/**********************************************************************/



void startOTA() {
  ArduinoOTA.onStart([]() {
    resetDisplay();
    lcd.println("Starting Update");
    lcd.display();
  });
  ArduinoOTA.onEnd([]() {
    resetDisplay();
    lcd.println("Success");
    lcd.display();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    resetDisplay();
    char progressPercent[16];
    sprintf(progressPercent,"%u%%",(progress / (total / 100)));
    lcd.println("Updating");
    lcd.setCursor(0, 16);
    lcd.print("Progress:");
    lcd.setCursor(0,48);
    justifyRight(progressPercent);
    lcd.display();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    resetDisplay();
    lcd.printf("Error[%u]: ", error);
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
    lcd.display();
  });
  ArduinoOTA.begin();
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


/**
 * @brief Runs once before loop, then passes to control to loop()
 * 
 */
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


/**
 * @brief Main routine of the program, loops forever.
 * 
 */
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
    calibrateScale(scale_data.knownWeight);
  }

  if (getScaleData.isReady()) {
    getScaleData.run();
  }

  if (updatedisplay.isReady()) {
    updatedisplay.run();
  }

  yield();
}