void Charging_Algorithm(){
Serial.print("hello world ");

  // Example: Sweep duty cycle up & down
  for (int d = 0; d <= 1023; d++) {
    ledcWrite(PWM_CHANNEL, d);
    delayMicroseconds(100);
  }
  for (int d = 1023; d >= 0; d--) {
    ledcWrite(PWM_CHANNEL, d);
    delayMicroseconds(100);

}
