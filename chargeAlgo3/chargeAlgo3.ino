volatile float vPV  = 0.0;
volatile float vOUT = 0.0;

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

// ---------------- SAFE PIN CONFIG (ESP32 CLASSIC) ----------------
#define HIN_PIN 25
#define LIN_PIN 26
#define SD_PIN  27

// I2C default pins15

#define SDA_PIN 21
#define SCL_PIN 22

// ---------------- PWM CONFIG ----------------
const int pwmFreq = 50000;
const int pwmResolution = 10;

// ---------------- CONTROL VARIABLES ----------------
float dutyPercent = 10.0;
float Vset = 15.0;

// ---------------- VOLTAGE DIVIDER ----------------
float R1_1 = 1000000.0;
float R2_1 = 20000.0;

float R1_2 = 1000000.0;
float R2_2 = 20000.0;

float ratio1;
float ratio2;

void setup() {

  Serial.begin(115200);
  delay(1000);

  // ---------------- I2C START ----------------
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setTimeOut(50);

  if (!ads.begin()) {
    Serial.println("ADS1115 not found!");
    while (1);
  }

  ads.setGain(GAIN_ONE);

  ratio1 = (R1_1 + R2_1) / R2_1;
  ratio2 = (R1_2 + R2_2) / R2_2;

  // ---------------- DRIVER PINS ----------------
  pinMode(SD_PIN, OUTPUT);
  digitalWrite(SD_PIN, HIGH);

  pinMode(LIN_PIN, OUTPUT);
  digitalWrite(LIN_PIN, LOW);

  // ---------------- PWM ----------------
  ledcAttach(HIN_PIN, pwmFreq, pwmResolution);

  Serial.println("Closed Loop Buck Control Ready");
  sheetsInit();
}

void loop() {

  // -------- SERIAL INPUT --------
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    float newSet = input.toFloat();
    if (newSet > 0) {
      Vset = newSet;
      Serial.print("New Target Voltage: ");
      Serial.println(Vset);
    }
  }

  // -------- READ ADS1115 --------
  int16_t adcPV  = ads.readADC_SingleEnded(0);
  int16_t adcOUT = ads.readADC_SingleEnded(1);

  vPV  = (adcPV  * 0.125 / 1000.0) * ratio1;
  vOUT = (adcOUT * 0.125 / 1000.0) * ratio2;

  sendToSheets(vPV, vOUT, iSolar, iOut);

  if (vPV < 1.0) return;

  float effectiveSet = min(Vset, vPV);
  float error = effectiveSet - vOUT;

  dutyPercent += error * 0.3;

  if (dutyPercent > 95) dutyPercent = 95;
  if (dutyPercent < 2)  dutyPercent = 2;

  uint32_t maxDuty = (1 << pwmResolution) - 1;
  uint32_t dutyValue = (dutyPercent * maxDuty) / 100;

  ledcWrite(HIN_PIN, dutyValue);

  Serial.print("PV: ");
  Serial.print(vPV, 2);
  Serial.print(" V | OUT: ");
  Serial.print(vOUT, 2);
  Serial.print(" V | Set: ");
  Serial.print(Vset, 2);  
  Serial.print(" V | Duty: ");
  Serial.println(dutyPercent, 2);

  delay(10);
}