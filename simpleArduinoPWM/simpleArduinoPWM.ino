//==============================
// IR2104 / IR2111 SIMPLE TEST
//==============================

// Connect IN pin of driver to Arduino pin 9
#define IN_PIN  7

// Shutdown pin (SD). HIGH = Driver Enabled
#define SD_PIN  8    

void setup() {
  pinMode(IN_PIN, OUTPUT);
  pinMode(SD_PIN, OUTPUT);

  digitalWrite(SD_PIN, HIGH);  // Enable IR2104/IR2111
}

void loop() {
  digitalWrite(IN_PIN, HIGH);   // HIGH-SIDE ON
  delay(10);

  digitalWrite(IN_PIN, LOW);    // LOW-SIDE ON
  delay(10);
}


//======================================
// IR2104 / IR2111 PWM TEST (Arduino)
//======================================

// #define IN_PIN   9   // PWM output to IN pin of driver
// #define SD_PIN   8   // Shutdown (HIGH = enable)

// int duty = 128;      // 50% duty cycle (0–255)

// void setup() {
//   Serial.begin(9600);

//   pinMode(SD_PIN, OUTPUT);
//   digitalWrite(SD_PIN, HIGH);  // Enable driver

//   pinMode(IN_PIN, OUTPUT);
// }

// void loop() {
//   Serial.println("IR2104/IR2111 PWM Running...");

//   // Sweep duty UP
//   for (duty = 20; duty < 230; duty += 5) {
//     analogWrite(IN_PIN, duty);
//     delay(20);
//   }

//   // Sweep duty DOWN
//   for (duty = 230; duty > 20; duty -= 5) {
//     analogWrite(IN_PIN, duty);
//     delay(20);
//   }

//   delay(200);
// }
