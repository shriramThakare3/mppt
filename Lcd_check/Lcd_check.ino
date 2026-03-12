#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

/* ================= LCD ================= */
LiquidCrystal_I2C lcd(0x27, 20, 4);

/* ================= BUTTONS ================= */
#define buttonRight   4
#define buttonLeft    5
#define buttonBack    6
#define buttonSelect  7

/* ================= EEPROM ================= */
#define EEPROM_SIZE 64              // Dummy EEPROM size
#define ADDR_BATT_MAX_V 0           // float (4 bytes)
#define ADDR_BACKLIGHT_MODE 8       // int (4 bytes)

/* ================= MENU ================= */
int menuPage = 0;
int settingMode = 0;
int setMenuPage = 0;
int subMenuPage = 0;

/* ================= STORED VALUES ================= */
float voltageBatteryMax = 14.40;    // DEFAULT VALUE
int backlightSleepMode  = 1;        // DEFAULT VALUE

/* TEMP BACKUP */
float floatTemp;
int intTemp;

/* ================= TIMING ================= */
unsigned long prevLCDMillis = 0;
const unsigned long lcdInterval = 300;

/* ================================================= */

void saveSettings() {
  EEPROM.put(ADDR_BATT_MAX_V, voltageBatteryMax);
  EEPROM.put(ADDR_BACKLIGHT_MODE, backlightSleepMode);
  EEPROM.commit();
}

void loadSettings() {
  EEPROM.get(ADDR_BATT_MAX_V, voltageBatteryMax);
  EEPROM.get(ADDR_BACKLIGHT_MODE, backlightSleepMode);

  // SAFETY CHECKS
  if (isnan(voltageBatteryMax) || voltageBatteryMax < 10 || voltageBatteryMax > 60)
    voltageBatteryMax = 14.40;

  if (backlightSleepMode < 0 || backlightSleepMode > 9)
    backlightSleepMode = 1;
}

/* ================= LCD ================= */

void showMainMenu() {
  lcd.setCursor(0,0);
  lcd.print("P: ");
  lcd.print(voltageBatteryMax);
  lcd.print("V ");
 
  lcd.print("5");
  lcd.print("A ");


  lcd.setCursor(0,1);
 lcd.print("B: ");
  lcd.print(voltageBatteryMax+1);
  lcd.print("V ");
 
  lcd.print("8");
  lcd.print("A ");

  
  lcd.setCursor(0,2);
  lcd.print("BKLIGHT: ");
  lcd.print(backlightSleepMode);
  lcd.print("   ");

  lcd.setCursor(0,3);
  lcd.print("SELECT = SETTINGS");
}

void settingsMenu() {

  if (!digitalRead(buttonRight)) { subMenuPage++; delay(200); }
  if (!digitalRead(buttonLeft))  { subMenuPage--; delay(200); }
  if (!digitalRead(buttonBack))  { settingMode = 0; lcd.clear(); delay(200); }

  if (subMenuPage < 0) subMenuPage = 1;
  if (subMenuPage > 1) subMenuPage = 0;

  /* ===== SUB MENU 0 : BATTERY MAX VOLTAGE ===== */
  if (subMenuPage == 0) {
    lcd.setCursor(0,0);
    lcd.print("MAX BATTERY V   ");

    if (setMenuPage == 0) {
      lcd.setCursor(0,1);
      lcd.print("= ");
      lcd.print(voltageBatteryMax,2);
      lcd.print("V        ");

      if (!digitalRead(buttonSelect)) {
        floatTemp = voltageBatteryMax;
        setMenuPage = 1;
        delay(300);
      }
    } else {
      lcd.setCursor(0,1);
      lcd.print("> ");
      lcd.print(voltageBatteryMax,2);
      lcd.print("V        ");

      if (!digitalRead(buttonRight)) { voltageBatteryMax += 0.1; delay(150); }
      if (!digitalRead(buttonLeft))  { voltageBatteryMax -= 0.1; delay(150); }

      if (!digitalRead(buttonBack)) {
        voltageBatteryMax = floatTemp;
        setMenuPage = 0;
        delay(200);
      }
      if (!digitalRead(buttonSelect)) {
        saveSettings();
        setMenuPage = 0;
        delay(200);
      }
    }
  }

  /* ===== SUB MENU 1 : BACKLIGHT MODE ===== */
  else if (subMenuPage == 1) {
    lcd.setCursor(0,0);
    lcd.print("BACKLIGHT SLEEP ");

    if (setMenuPage == 0) {
      lcd.setCursor(0,1);
      lcd.print("= MODE ");
      lcd.print(backlightSleepMode);
      lcd.print("        ");

      if (!digitalRead(buttonSelect)) {
        intTemp = backlightSleepMode;
        setMenuPage = 1;
        delay(300);
      }
    } else {
      lcd.setCursor(0,1);
      lcd.print("> MODE ");
      lcd.print(backlightSleepMode);
      lcd.print("        ");

      if (!digitalRead(buttonRight)) { backlightSleepMode++; delay(150); }
      if (!digitalRead(buttonLeft))  { backlightSleepMode--; delay(150); }

      backlightSleepMode = constrain(backlightSleepMode, 0, 9);

      if (!digitalRead(buttonBack)) {
        backlightSleepMode = intTemp;
        setMenuPage = 0;
        delay(200);
      }
      if (!digitalRead(buttonSelect)) {
        saveSettings();
        setMenuPage = 0;
        delay(200);
      }
    }
  }
}

/* ================================================= */

void setup() {
  Wire.begin(8, 9);
  lcd.init();
  lcd.backlight();

  pinMode(buttonRight, INPUT_PULLUP);
  pinMode(buttonLeft, INPUT_PULLUP);
  pinMode(buttonBack, INPUT_PULLUP);
  pinMode(buttonSelect, INPUT_PULLUP);

  EEPROM.begin(EEPROM_SIZE);
  loadSettings();

  lcd.clear();
}

void loop() {

  if (!settingMode) {
    if (!digitalRead(buttonSelect)) {
      settingMode = 1;
      lcd.clear();
      delay(300);
    }

    if (millis() - prevLCDMillis > lcdInterval) {
      prevLCDMillis = millis();
      showMainMenu();
    }
  } else {
    settingsMenu();
  }
}
