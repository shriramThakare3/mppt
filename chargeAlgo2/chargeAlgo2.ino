#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

// PWM PINS
#define HIN_PIN 5
#define LIN_PIN 6
#define SD_PIN  4

const int pwmFreq = 20000;
const int pwmResolution = 10;

float dutyPercent = 10;

// Voltage divider resistors
float R1_1 = 1000000.0;  // PV
float R2_1 = 20000.0;

float R1_2 = 1000000.0;  // Output
float R2_2 = 20000.0;

float ratio1;
float ratio2;

float Vset = 15.0;   // default output target

void setup() {

  Serial.begin(115200);

  Wire.begin(8, 9);

  if (!ads.begin()) {
    Serial.println("ADS1115 not found!");
    while (1);
  }

  ads.setGain(GAIN_ONE);

  ratio1 = (R1_1 + R2_1) / R2_1;
  ratio2 = (R1_2 + R2_2) / R2_2;

  pinMode(SD_PIN, OUTPUT);
  digitalWrite(SD_PIN, HIGH);

  pinMode(LIN_PIN, OUTPUT);
  digitalWrite(LIN_PIN, LOW);

  ledcAttach(HIN_PIN, pwmFreq, pwmResolution);

  Serial.println("Closed Loop Buck Control Ready");
  Serial.println("Enter desired Output Voltage:");
}

void loop() {

  // -------- Read Serial for Vset --------
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      float newSet = input.toFloat();
      if (newSet > 0) {
        Vset = newSet;
        Serial.print("New Target Voltage: ");
        Serial.println(Vset);
      }
    }
  }

  // -------- Read ADS --------
  int16_t adcPV = ads.readADC_SingleEnded(0);
  int16_t adcOUT = ads.readADC_SingleEnded(1);

  float vPV = (adcPV * 0.125 / 1000.0) * ratio1;
  float vOUT = (adcOUT * 0.125 / 1000.0) * ratio2;

  // -------- Safety: Buck cannot boost --------
  if (Vset > vPV) {
    Vset = vPV;
  }

  // -------- Calculate Ideal Duty --------
  float idealDuty = (Vset / vPV) * 100.0;

  // Limit duty
  if (idealDuty > 95) idealDuty = 95;
  if (idealDuty < 2) idealDuty = 2;

  // -------- Small Feedback Correction --------
  float error = Vset - vOUT;

  dutyPercent += error * 0.5;   // Proportional gain

  if (dutyPercent > 95) dutyPercent = 95;
  if (dutyPercent < 2) dutyPercent = 2;

  // -------- Apply PWM --------
  uint32_t maxDuty = (1 << pwmResolution) - 1;
  uint32_t dutyValue = (dutyPercent * maxDuty) / 100;
  ledcWrite(HIN_PIN, dutyValue);

  // -------- Debug --------
  Serial.print("PV: "); Serial.print(vPV, 2);
  Serial.print(" V | OUT: "); Serial.print(vOUT, 2);
  Serial.print(" V | Set: "); Serial.print(Vset, 2);
  Serial.print(" V | Duty: "); Serial.print(dutyPercent, 2);
  Serial.println(" %");

  delay(10);
}
