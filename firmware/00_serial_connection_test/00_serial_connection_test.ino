void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ESP32 connection test");
  Serial.println("Serial communication is working");
}

void loop() {
  static unsigned long lastPrintTime = 0;

  if (millis() - lastPrintTime >= 2000) {
    lastPrintTime = millis();
    Serial.println("ESP32 is running");
  }
}