
#include <Arduino.h>

// Buck converter MOSFET control pin
const int mosfetPin = 25;  // Connect your MOSFET gate to this pin

// PWM parameters
const int pwmChannel = 0;   // LEDC channel
const int pwmFreq = 50000;  // 50 kHz (typical for buck converter)
const int pwmResolution = 12; // 12-bit resolution (0-4095)

// Shared variable between cores
volatile int sharedCounter = 0;



#define PWM_PIN       25      // Your MOSFET driver input pin
#define PWM_CHANNEL   0       // LEDC channel (0–7)
#define PWM_FREQ      50000   // 50 kHz switching frequency
#define PWM_RES       10      // 10-bit resolution (0–1023 steps)

int dutyCycle = 512;          // Start at 50% duty (range: 0–1023)



//=========================================================
// Task for Core 0 (Real-time / control)
void core0Task(void * parameter){
  while(true){
    Charging_Algorithm():
    sendTOSheets(vPV,vOUT);
  }
}

// Task for Core 1 (Background / monitoring)
void core1Task(void * parameter){
  while(true){
    Charging_Algorithm();   
    sendToSheets(panelVoltage, panelCurrent, batVoltage, batCurrent); 
 
  }
}


//=============================================================
void setup() {
  Serial.begin(115200);
  
 // Configure PWM
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);

  // Set initial duty cycle
  ledcWrite(PWM_CHANNEL, dutyCycle);
  
  sheetsInit();




//====================================================================

  xTaskCreatePinnedToCore(
    core1Task,       // Task function
    "Core1 Task",    // Name
    4096,            // Stack size
    NULL,            // Parameter
    1,               // Priority
    NULL,            // Task handle
    1                // Core ID (1)
  );

  // Launch core0Task on Core 0 (pinning to core 0)
  xTaskCreatePinnedToCore(
    core0Task,
    "Core0 Task",
    4096,
    NULL,
    1,
    NULL,
    0
  );
}

void loop() {
  // Empty loop; all tasks are handled on cores
}
