#include <Arduino.h>

#define RXD2 16 // Подключен к PA9 (STM32)
#define TXD2 17 // Подключен к PA10 (STM32)

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  
  Serial.println("\n=== TELEMETRY LOGGER (NO REMAP / Native PA9/PA10) ===");
}

void loop() {
  if (Serial2.available()) {
    String inputData = Serial2.readStringUntil('\n');
    inputData.trim();
    if (inputData.length() > 0) {
      Serial.println(inputData);
    }
  }
}
