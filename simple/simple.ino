 // Simple IR2104 test
// HO and LO will switch alternately

#define IN_PIN  25  // PWM/IO pin connected to IR2104 IN
#define SD_PIN  26   // Shutdown pin (set HIGH = enable)

void setup() {
  pinMode(IN_PIN, OUTPUT);
  pinMode(SD_PIN, OUTPUT);

  digitalWrite(SD_PIN, HIGH);  // enable driver
}

void loop() {  
  digitalWrite(IN_PIN, HIGH);   // High-side ON, Low-side OFF
  delay(10);

  digitalWrite(IN_PIN, LOW);    // High-side OFF, Low-side ON
  delay(10); 
}
 
// ESP32 IR2104 Driver Test using NEW LEDC API

//====================================================================================================

//====================================================================================================

// #define IN_PIN        15    // IR2104 IN pin
// #define SD_PIN        4      // IR2104 Shutdown pin (HIGH = enable)

// #define PWM_FREQ      100   // 1 kHz test PWM
// #define PWM_RES       8      // 8-bit resolution (0–255)

// int duty = 128;             // 50% duty

// void setup() {
//   Serial.begin(115200);

//   // Enable IR2104
//   pinMode(SD_PIN, OUTPUT);
//   digitalWrite(SD_PIN, HIGH);

//   // Attach PWM to IN_PIN using the NEW API
//   ledcAttach(IN_PIN, PWM_FREQ, PWM_RES);

//   // Start 50% PWM
//   ledcWrite(IN_PIN, duty);
// }

// void loop() {
//   Serial.println("IR2104 PWM Test Running...");

//   // Sweep duty cycle slow for demo
//   for (duty = 20; duty < 230; duty += 5) {
//     ledcWrite(IN_PIN, duty);
//   }

//   for (duty = 230; duty > 20; duty -= 5) {
//     ledcWrite(IN_PIN, duty);
//     delay(30);
//   }
//     delay(100);
// }
