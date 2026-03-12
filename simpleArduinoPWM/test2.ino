// Arduino UNO version — synchronous buck PWM demo
// NOTE: This also has NO dead-time and is only for bench LED/driver demo,
// NOT for real MOSFET buck without dead-time!

#define HIGH_SIDE_PIN 9     // PWM pin
#define LOW_SIDE_PIN  10    // PWM pin

// On UNO: analogWrite() is ~490 Hz on most pins, NOT 50kHz!
// Resolution is 8-bit: 0–255
int duty = 128; // 50%

void setup() {
  Serial.begin(115200);

  pinMode(HIGH_SIDE_PIN, OUTPUT);
  pinMode(LOW_SIDE_PIN, OUTPUT);

  // Start complementary outputs
  analogWrite(HIGH_SIDE_PIN, duty);
  analogWrite(LOW_SIDE_PIN, 255 - duty);
}

void loop() {
  Serial.println("hello");

  for (duty = 25; duty < 230; duty += 5) {
    analogWrite(HIGH_SIDE_PIN, duty);
    analogWrite(LOW_SIDE_PIN, 255 - duty);
    delay(50);
  }
}
