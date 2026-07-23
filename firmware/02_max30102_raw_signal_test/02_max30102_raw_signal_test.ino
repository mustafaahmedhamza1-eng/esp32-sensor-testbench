#include <Wire.h>
#include <U8g2lib.h>
#include "MAX30105.h"

// OLED wiring currently soldered in the physical prototype.
constexpr uint8_t OLED_SDA_PIN = 21;
constexpr uint8_t OLED_SCL_PIN = 22;

// Example sensor test port.
// Modify these pins to match your ESP32 hardware.
constexpr uint8_t SENSOR_SDA_PIN = 32;
constexpr uint8_t SENSOR_SCL_PIN = 33;

constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint8_t MAX30102_ADDRESS = 0x57;

constexpr unsigned long SAMPLE_INTERVAL_MS = 250;
constexpr uint16_t TEST_SAMPLE_COUNT = 20;

// Separate I2C controller for the MAX30102.
TwoWire sensorBus = TwoWire(1);

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
    U8G2_R0,
    U8X8_PIN_NONE
);

MAX30105 maxSensor;

bool oledReady = false;
bool sensorReady = false;
bool summaryPrinted = false;

uint16_t sampleNumber = 0;
uint16_t validSamples = 0;

uint32_t minimumRed = UINT32_MAX;
uint32_t maximumRed = 0;
uint32_t minimumIR = UINT32_MAX;
uint32_t maximumIR = 0;

unsigned long lastSampleTime = 0;

bool probeAddress(TwoWire &bus, uint8_t address) {
  bus.beginTransmission(address);
  return bus.endTransmission() == 0;
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

void showReading(uint32_t redValue, uint32_t irValue) {
  if (!oledReady) {
    return;
  }

  char redText[24];
  char irText[24];
  char sampleText[24];

  snprintf(
      redText,
      sizeof(redText),
      "RED: %lu",
      static_cast<unsigned long>(redValue)
  );

  snprintf(
      irText,
      sizeof(irText),
      "IR : %lu",
      static_cast<unsigned long>(irValue)
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

  oled.drawStr(0, 12, "MAX30102 Raw Test");
  oled.drawStr(0, 29, redText);
  oled.drawStr(0, 44, irText);
  oled.drawStr(0, 60, sampleText);

  oled.sendBuffer();
}

void printSummary() {
  if (summaryPrinted) {
    return;
  }

  summaryPrinted = true;

  Serial.println();
  Serial.println("================================");
  Serial.println("MAX30102 Test Summary");
  Serial.println("================================");

  Serial.print("Valid samples: ");
  Serial.print(validSamples);
  Serial.print("/");
  Serial.println(TEST_SAMPLE_COUNT);

  if (validSamples > 0) {
    Serial.print("RED range: ");
    Serial.print(minimumRed);
    Serial.print(" to ");
    Serial.println(maximumRed);

    Serial.print("IR range: ");
    Serial.print(minimumIR);
    Serial.print(" to ");
    Serial.println(maximumIR);
  }

  if (validSamples == TEST_SAMPLE_COUNT) {
    Serial.println("[PASS] MAX30102 produced valid raw optical data");
    Serial.println("Overall result: PASS");

    showMessage(
        "MAX30102 TEST",
        "RESULT: PASS",
        "20 valid samples"
    );
  } else if (validSamples > 0) {
    Serial.println("[WARN] Some samples were invalid");
    Serial.println("Overall result: PARTIAL PASS");

    showMessage(
        "MAX30102 TEST",
        "PARTIAL PASS",
        "Check placement"
    );
  } else {
    Serial.println("[FAIL] No valid raw optical data was received");
    Serial.println("Overall result: FAIL");

    showMessage(
        "MAX30102 TEST",
        "RESULT: FAIL",
        "No optical data"
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
  Serial.println("ESP32 MAX30102 Raw Signal Test");
  Serial.println("================================");

  // Start the OLED I2C bus.
  Wire.begin(
      OLED_SDA_PIN,
      OLED_SCL_PIN,
      100000
  );

  if (probeAddress(Wire, OLED_ADDRESS)) {
    oled.setI2CAddress(OLED_ADDRESS << 1);
    oled.begin();

    oledReady = true;

    Serial.println("[PASS] OLED detected at address 0x3C");

    showMessage(
        "MAX30102 TEST",
        "Initializing...",
        "Place finger gently"
    );
  } else {
    Serial.println("[WARN] OLED was not detected");
    Serial.println("The sensor test will continue without the display.");
  }

  // Start the separate I2C bus used by the soldered MAX30102.
  sensorBus.begin(
      SENSOR_SDA_PIN,
      SENSOR_SCL_PIN,
      400000
  );

  Serial.println("Scanning MAX30102 I2C bus");

  if (!probeAddress(sensorBus, MAX30102_ADDRESS)) {
    Serial.println("[FAIL] MAX30102 was not detected at address 0x57");
    Serial.println("Check power, ground, SDA, and SCL.");

    showMessage(
        "MAX30102 TEST",
        "RESULT: FAIL",
        "Sensor not found"
    );

    return;
  }

  Serial.println("[PASS] MAX30102 detected at address 0x57");

  if (!maxSensor.begin(sensorBus, I2C_SPEED_FAST)) {
    Serial.println("[FAIL] MAX30102 library initialization failed");

    showMessage(
        "MAX30102 TEST",
        "RESULT: FAIL",
        "Initialization"
    );

    return;
  }

  maxSensor.setup(
      0x1F,  // LED brightness
      4,     // Sample averaging
      2,     // Red and IR mode
      100,   // Sample rate
      411,   // Pulse width
      4096   // ADC range
  );

  sensorReady = true;

  Serial.println("[PASS] MAX30102 initialization completed");
  Serial.println("[INFO] Place a finger gently on the sensor");
  Serial.println();
  Serial.println("Sample,RED,IR");
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

  uint32_t redValue = maxSensor.getRed();
  uint32_t irValue = maxSensor.getIR();

  sampleNumber++;

  Serial.print(sampleNumber);
  Serial.print(",");
  Serial.print(redValue);
  Serial.print(",");
  Serial.println(irValue);

  showReading(redValue, irValue);

  if (redValue > 0 && irValue > 0) {
    validSamples++;

    minimumRed = min(minimumRed, redValue);
    maximumRed = max(maximumRed, redValue);

    minimumIR = min(minimumIR, irValue);
    maximumIR = max(maximumIR, irValue);
  }

  if (sampleNumber >= TEST_SAMPLE_COUNT) {
    printSummary();
  }
}