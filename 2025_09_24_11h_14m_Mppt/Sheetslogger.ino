// ================================================================
//  ☀️  SOLAR MPPT — Google Sheets WiFi Logger
//  Add this to your existing MPPT sketch from:
//  https://github.com/shriramThakare3/mppt
//
//  INSTRUCTIONS:
//  1. Copy the #include and config section into your main .ino
//  2. Call sheetsInit() inside your setup()
//  3. Call sendToSheets(sv, sc, ov, oc) wherever you read sensors
//  4. Set your WiFi + Web App URL below
// ================================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ----------------------------------------------------------------
//  ⚙️  CHANGE THESE 3 LINES — nothing else needs to change
// ----------------------------------------------------------------
const char* WIFI_SSID      = "ST";
const char* WIFI_PASSWORD  = "ssssssss";
const char* SHEETS_URL     = "https://script.google.com/macros/s/AKfycbwT4rTXDFyXQwFFHKRgq5ejgKpweoZ0yTCSovPivjLT1_p0f3pPOGouYW-2UfVAvrYvqA/exec";
//  ↑ Get this URL from: Apps Script → Deploy → Manage Deployments
// ----------------------------------------------------------------

// How often to send data (milliseconds). 10000 = every 10 seconds
#define SEND_INTERVAL_MS  10000

// ----------------------------------------------------------------
//  YOUR EXISTING PIN DEFINITIONS (match your repo's pin names)
//  Check your sensors.ino or main .ino — replace if different
// ----------------------------------------------------------------
#define PIN_SOLAR_VOLTAGE   34    // ADC — solar panel voltage divider
#define PIN_SOLAR_CURRENT   35    // ADC — solar panel ACS712/INA
#define PIN_OUTPUT_VOLTAGE  32    // ADC — battery/output voltage divider
#define PIN_OUTPUT_CURRENT  33    // ADC — output ACS712/INA

// Voltage divider ratio (R1+R2)/R2
// Example: 100kΩ + 10kΩ → ratio = (100+10)/10 = 11.0
// Measure your real voltage and adjust until readings match multimeter
#define SOLAR_V_RATIO   11.0f
#define OUTPUT_V_RATIO  11.0f

// ACS712 sensitivity (V/A) and zero-current ADC count
// ACS712-5A  → 0.185 V/A
// ACS712-20A → 0.100 V/A
// ACS712-30A → 0.066 V/A  ← most common for solar
#define CURRENT_SENS    0.066f
#define CURRENT_ZERO    1865      // Read ADC at 0A and put that value here

// ADC averaging — more samples = more stable reading
#define ADC_SAMPLES     20

// ----------------------------------------------------------------
//  INTERNAL STATE — don't change
// ----------------------------------------------------------------
static unsigned long _lastSendMs  = 0;
static bool          _wifiReady   = false;
static uint32_t      _sendCount   = 0;
static uint32_t      _failCount   = 0;


// ================================================================
//  CALL THIS ONCE in your setup()
// ================================================================
void sheetsInit() {
  Serial.println(F("\n☀️  Solar MPPT — Google Sheets Logger"));
  Serial.println(F("--------------------------------------"));

  analogReadResolution(12);        // ESP32: 12-bit ADC (0–4095)
  analogSetAttenuation(ADC_11db);  // Full range 0–3.3V

  Serial.print(F("Connecting to WiFi: "));
  Serial.print(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print('.');
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    _wifiReady = true;
    Serial.println(F(" ✅"));
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F(" ❌ WiFi failed. Will retry in loop."));
  }
  Serial.println(F("--------------------------------------\n"));
}


// ================================================================
//  CALL THIS in your loop() — reads sensors & sends to Sheets
//  Pass your already-read values, OR let it read pins itself
// ================================================================

// ── Option A: Pass your existing sensor variables directly ──────
//    Use this if your code already reads voltages/currents
void sendToSheets(float solarV, float solarA, float outputV, float outputA) {
  if (millis() - _lastSendMs < SEND_INTERVAL_MS) return;
  _lastSendMs = millis();
  _doSend(solarV, solarA, outputV, outputA);
}

// ── Option B: Let this code read the ADC pins itself ────────────
//    Use this if you haven't read sensors yet
void sendToSheetsAutoRead() {
  if (millis() - _lastSendMs < SEND_INTERVAL_MS) return;
  _lastSendMs = millis();

  float sv = _readVoltage(PIN_SOLAR_VOLTAGE,  SOLAR_V_RATIO);
  float sc = _readCurrent(PIN_SOLAR_CURRENT);
  float ov = _readVoltage(PIN_OUTPUT_VOLTAGE, OUTPUT_V_RATIO);
  float oc = _readCurrent(PIN_OUTPUT_CURRENT);

  _doSend(sv, sc, ov, oc);
}


// ================================================================
//  INTERNAL: build URL and POST to Google Sheets
// ================================================================
void _doSend(float sv, float sc, float ov, float oc) {

  // Reconnect WiFi if dropped
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("⚠️  WiFi lost — reconnecting..."));
    WiFi.reconnect();
    uint8_t w = 0;
    while (WiFi.status() != WL_CONNECTED && w++ < 20) delay(500);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("❌  Reconnect failed, skipping this reading."));
      _failCount++;
      return;
    }
  }

  // Print to Serial Monitor
  float ip  = sv * sc;
  float op  = ov * oc;
  float eff = (ip > 0) ? min(100.0f, op / ip * 100.0f) : 0;

  Serial.printf("📡 Sending #%lu | SV=%.2fV  SC=%.2fA  OV=%.2fV  OC=%.2fA  Pin=%.1fW  Pout=%.1fW  Eff=%.1f%%\n",
    ++_sendCount, sv, sc, ov, oc, ip, op, eff);

  // Build GET URL with parameters (works without ArduinoJson library)
  // Google Apps Script accepts both GET and POST
  char url[512];
  snprintf(url, sizeof(url),
    "%s?sv=%.2f&sc=%.2f&ov=%.2f&oc=%.2f",
    SHEETS_URL, sv, sc, ov, oc);

  WiFiClientSecure client;
  client.setInsecure();  // Skip SSL cert check (fine for Apps Script)

  HTTPClient http;
  http.begin(client, url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);  // ← REQUIRED for Apps Script
  http.setTimeout(10000);  // 10 second timeout

  int code = http.GET();

  if (code == 200 || code == 302) {
    String resp = http.getString();
    if (resp.indexOf("true") >= 0) {
      Serial.println(F("✅  Logged to Sheets successfully"));
    } else {
      Serial.print(F("⚠️  Sheets response: "));
      Serial.println(resp.substring(0, 80));
    }
  } else if (code > 0) {
    Serial.printf("⚠️  HTTP code: %d\n", code);
    _failCount++;
  } else {
    Serial.printf("❌  HTTP error: %s\n", http.errorToString(code).c_str());
    _failCount++;
  }

  http.end();

  // Print stats every 10 sends
  if (_sendCount % 10 == 0) {
    Serial.printf("📊 Stats: %lu sent, %lu failed, %.1f%% success\n",
      _sendCount, _failCount,
      _sendCount > 0 ? (float)(_sendCount - _failCount) / _sendCount * 100 : 0);
  }
}


// ================================================================
//  SENSOR READING HELPERS
//  Only needed if using sendToSheetsAutoRead()
// ================================================================
float _readVoltage(uint8_t pin, float ratio) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  float adcCounts = sum / (float)ADC_SAMPLES;
  float adcVolts  = adcCounts * 3.3f / 4095.0f;
  return adcVolts * ratio;
}

float _readCurrent(uint8_t pin) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  float adcCounts = sum / (float)ADC_SAMPLES;
  float adcVolts  = adcCounts * 3.3f / 4095.0f;
  float zeroVolts = CURRENT_ZERO * 3.3f / 4095.0f;
  float current   = (adcVolts - zeroVolts) / CURRENT_SENS;
  return max(0.0f, current);
}


// ================================================================
//  HOW TO USE IN YOUR EXISTING CODE
//  ─────────────────────────────────────────────────────────────
//
//  In your main .ino (e.g. 2025_09_24_11h_14m_Mppt.ino):
//
//  ① Add at the top:
//       #include "SheetsLogger.h"
//     OR just paste this whole file as a new tab in Arduino IDE
//
//  ② In setup(), add ONE line:
//       void setup() {
//         ...your existing setup code...
//         sheetsInit();   // ← ADD THIS
//       }
//
//  ③ In loop(), add ONE line after your sensor reads:
//
//     IF your code already reads solar_voltage, solar_current etc:
//       void loop() {
//         ...your MPPT algorithm...
//         // After your sensor read block:
//         sendToSheets(solar_voltage, solar_current, bat_voltage, bat_current);
//       }
//
//     IF you want this file to read sensors itself:
//       void loop() {
//         ...your MPPT algorithm...
//         sendToSheetsAutoRead();
//       }
//
//  ④ That's it! Open Serial Monitor at 115200 baud to see logs.
// ================================================================


// ================================================================
//  STANDALONE TEST SKETCH
//  If you want to test WiFi + Sheets BEFORE integrating with MPPT,
//  comment out everything above and uncomment this section:
// ================================================================

/*
void setup() {
  Serial.begin(115200);
  sheetsInit();
}

void loop() {
  // Simulate sensor values — replace with your real reads
  float sv = 36.5 + random(-10, 10) * 0.1;   // Solar voltage
  float sc =  7.2 + random(-5,  5) * 0.1;    // Solar current
  float ov = 24.1 + random(-3,  3) * 0.1;    // Output voltage
  float oc =  9.8 + random(-5,  5) * 0.1;    // Output current

  sendToSheets(sv, sc, ov, oc);
  delay(100);  // small delay, sendToSheets() handles the 10s interval
}
*/
