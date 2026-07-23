#include <Wire.h>
#include <U8g2lib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <float.h>

// OLED wiring in the physical prototype.
constexpr uint8_t OLED_SDA_PIN = 21;
constexpr uint8_t OLED_SCL_PIN = 22;

// DS18B20 wiring in the physical prototype.
constexpr uint8_t SENSOR_DATA_PIN = 32;

constexpr uint8_t OLED_ADDRESS = 0x3C;

constexpr uint8_t TEST_SAMPLE_COUNT = 10;
constexpr unsigned long SAMPLE_INTERVAL_MS = 1500;

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
    U8G2_R0,
    U8X8_PIN_NONE
);

OneWire oneWire(SENSOR_DATA_PIN);
DallasTemperature ds18b20(&oneWire);

bool oledReady = false;
bool sensorReady = false;
bool summaryPrinted = false;

uint8_t sampleNumber = 0;
uint8_t validSamples = 0;

float temperatureSum = 0.0;
float minimumTemperature = FLT_MAX;
float maximumTemperature = -FLT_MAX;

unsigned long lastSampleTime = 0;

bool probeI2CAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool isValidTemperature(float temperatureC) {
  if (temperatureC == DEVICE_DISCONNECTED_C) {
    return false;
  }

  return temperatureC >= -55.0 && temperatureC <= 125.0;
}

void showMessage(
    const char *line1,
    const char *line2,
    const char *line3
) {
  if (!oledReady) {
    return;
  }

  oled.clearBuffer();
  oled.setFont(u8g2_font_6x12_tr);

  oled.drawStr(0, 14, line1);
  oled.drawStr(0, 34, line2);
  oled.drawStr(0, 54, line3);

  oled.sendBuffer();
}

void showTemperature(float temperatureC) {
  if (!oledReady) {
    return;
  }

  char temperatureText[24];
  char sampleText[24];

  snprintf(
      temperatureText,
      sizeof(temperatureText),
      "Temp: %.2f C",
      temperatureC
  );

  snprintf(
      sampleText,
      sizeof(sampleText),
      "Sample: %u/%u",
      sampleNumber,
      TEST_SAMPLE_COUNT
  );

  oled.clearBuffer();
  oled.setFont(u8g2_font_6x12_tr);

  oled.drawStr(0, 13, "DS18B20 Test");
  oled.drawStr(0, 34, temperatureText);
  oled.drawStr(0, 55, sampleText);

  oled.sendBuffer();
}

void printSummary() {
  if (summaryPrinted) {
    return;
  }

  summaryPrinted = true;

  Serial.println();
  Serial.println("================================");
  Serial.println("DS18B20 Test Summary");
  Serial.println("================================");

  Serial.print("Valid samples: ");
  Serial.print(validSamples);
  Serial.print("/");
  Serial.println(TEST_SAMPLE_COUNT);

  if (validSamples > 0) {
    float averageTemperature =
        temperatureSum / static_cast<float>(validSamples);

    Serial.print("Average temperature: ");
    Serial.print(averageTemperature, 2);
    Serial.println(" C");

    Serial.print("Temperature range: ");
    Serial.print(minimumTemperature, 2);
    Serial.print(" to ");
    Serial.print(maximumTemperature, 2);
    Serial.println(" C");
  }

  if (validSamples == TEST_SAMPLE_COUNT) {
    Serial.println("[PASS] DS18B20 produced valid temperature data");
    Serial.println("Overall result: PASS");

    showMessage(
        "DS18B20 TEST",
        "RESULT: PASS",
        "10 valid samples"
    );
  } else if (validSamples > 0) {
    Serial.println("[WARN] Some temperature samples were invalid");
    Serial.println("Overall result: PARTIAL PASS");

    showMessage(
        "DS18B20 TEST",
        "PARTIAL PASS",
        "Check connection"
    );
  } else {
    Serial.println("[FAIL] No valid temperature data was received");
    Serial.println("Overall result: FAIL");

    showMessage(
        "DS18B20 TEST",
        "RESULT: FAIL",
        "No valid data"
    );
  }

  Serial.println();
  Serial.println("Press the ESP32 RST button to run the test again.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 DS18B20 Temperature Test");
  Serial.println("================================");

  Wire.begin(
      OLED_SDA_PIN,
      OLED_SCL_PIN,
      100000
  );

  if (probeI2CAddress(OLED_ADDRESS)) {
    oled.setI2CAddress(OLED_ADDRESS << 1);
    oled.begin();

    oledReady = true;

    Serial.println("[PASS] OLED detected at address 0x3C");

    showMessage(
        "DS18B20 TEST",
        "Initializing...",
        "Please wait"
    );
  } else {
    Serial.println("[WARN] OLED was not detected");
    Serial.println("The sensor test will continue without the display.");
  }

  ds18b20.begin();

  uint8_t detectedSensors = ds18b20.getDeviceCount();

  Serial.print("DS18B20 sensors detected: ");
  Serial.println(detectedSensors);

  if (detectedSensors == 0) {
    Serial.println("[FAIL] DS18B20 was not detected");
    Serial.println("Check power, ground, data wire, and pull-up resistor.");

    showMessage(
        "DS18B20 TEST",
        "RESULT: FAIL",
        "Sensor not found"
    );

    return;
  }

  ds18b20.setResolution(12);

  sensorReady = true;

  Serial.println("[PASS] DS18B20 detected");
  Serial.println("[PASS] Sensor initialization completed");
  Serial.println();
  Serial.println("Sample,Temperature_C");
}

void loop() {
  if (!sensorReady || summaryPrinted) {
    return;
  }

  unsigned long currentTime = millis();

  if (currentTime - lastSampleTime < SAMPLE_INTERVAL_MS) {
    return;
  }

  lastSampleTime = currentTime;

  ds18b20.requestTemperatures();

  float temperatureC = ds18b20.getTempCByIndex(0);

  sampleNumber++;

  Serial.print(sampleNumber);
  Serial.print(",");

  if (isValidTemperature(temperatureC)) {
    Serial.println(temperatureC, 2);

    validSamples++;
    temperatureSum += temperatureC;

    minimumTemperature = min(
        minimumTemperature,
        temperatureC
    );

    maximumTemperature = max(
        maximumTemperature,
        temperatureC
    );

    showTemperature(temperatureC);
  } else {
    Serial.println("INVALID");

    showMessage(
        "DS18B20 TEST",
        "Invalid reading",
        "Check sensor"
    );
  }

  if (sampleNumber >= TEST_SAMPLE_COUNT) {
    printSummary();
  }
}