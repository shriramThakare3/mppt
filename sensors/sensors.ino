 #include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

#define SDA_PIN 8
#define SCL_PIN 9

// ----------- Sensor 1 Resistors (A0) -----------
float R1_1 = 1000000.0;   // Top resistor (ohms)
float R2_1 = 20000.0;     // Bottom resistor (ohms)

// ----------- Sensor 2 Resistors (A1) -----------
float R1_2 = 1000000.0;   // Top resistor (ohms)
float R2_2 = 20000.0;     // Bottom resistor (ohms)

float ratio1;
float ratio2;

void setup() {
  Serial.begin(115200);
  
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!ads.begin()) {
    Serial.println("ADS1115 not found!");
    while (1);
  }

  ads.setGain(GAIN_ONE);  // ±4.096V range

  // Calculate divider ratios
  ratio1 = (R1_1 + R2_1) / R2_1;
  ratio2 = (R1_2 + R2_2) / R2_2;

  Serial.println("ADS1115 Two-Channel Voltage Monitor Ready");
  Serial.print("Ratio 1: "); Serial.println(ratio1, 4);
  Serial.print("Ratio 2: "); Serial.println(ratio2, 4);
}

void loop() {

  // Read ADC channels
  int16_t adc0 = ads.readADC_SingleEnded(0);  // A0
  int16_t adc1 = ads.readADC_SingleEnded(1);  // A1

  // Convert to voltage (GAIN_ONE = 0.125mV per bit)
  float v0 = adc0 * 0.125 / 1000.0;
  float v1 = adc1 * 0.125 / 1000.0;

  // Calculate real voltages
  float realV0 = v0 * ratio1;
  float realV1 = v1 * ratio2;

  Serial.print("Sensor 1 (A0): ");
  Serial.print(realV0, 2);
  Serial.print(" V   |   ");

  Serial.print("Sensor 2 (A1): ");
  Serial.print(realV1, 2);
  Serial.println(" V");

  delay(1000);
}
