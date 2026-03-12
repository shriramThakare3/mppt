 #define HIN_PIN   5
#define LIN_PIN   6
#define SD_PIN    4

const int pwmFreq = 2000;     
const int pwmResolution = 10;  

int dutyPercent = 50;

void setup() {
  Serial.begin(115200);

  pinMode(SD_PIN, OUTPUT);
  digitalWrite(SD_PIN, HIGH);

  pinMode(LIN_PIN, OUTPUT);
  digitalWrite(LIN_PIN, LOW);

  // Attach PWM
  ledcAttach(HIN_PIN, pwmFreq, pwmResolution);

  // ---- SET INITIAL DUTY (50%) ----
  uint32_t maxDuty = (1 << pwmResolution) - 1;   // 1023 for 10-bit
  uint32_t dutyValue = (dutyPercent * maxDuty) / 100;

  ledcWrite(HIN_PIN, dutyValue);

  Serial.print("Initial Duty: ");
  Serial.print(dutyPercent);
  Serial.println("%");
  Serial.println("Enter Duty Cycle (0–100):");
}

void loop() {

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {

      int newDuty = input.toInt();

      if (newDuty >= 0 && newDuty <= 100) {
        dutyPercent = newDuty;

        uint32_t maxDuty = (1 << pwmResolution) - 1;
        uint32_t dutyValue = (dutyPercent * maxDuty) / 100;

        ledcWrite(HIN_PIN, dutyValue);

        Serial.print("Duty set to: ");
        Serial.print(dutyPercent);
        Serial.println("%");
      }
      else {
        Serial.println("Invalid input! Enter 0–100");
      }
    }
  }
}
