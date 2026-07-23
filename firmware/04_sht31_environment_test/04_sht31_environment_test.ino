#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_SHT31.h>
#include <float.h>
#include <math.h>

/*
  ESP32 SHT31 Temperature and Humidity Test

  OLED bus:
    SDA -> GPIO 21
    SCL -> GPIO 22

  SHT31 sensor test port:
    SDA -> GPIO 32
    SCL -> GPIO 33
*/

// OLED I2C bus.
constexpr uint8_t OLED_SDA_PIN = 21;
constexpr uint8_t OLED_SCL_PIN = 22;

// SHT31 I2C bus.
constexpr uint8_t SENSOR_SDA_PIN = 32;
constexpr uint8_t SENSOR_SCL_PIN = 33;

constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint8_t SHT31_ADDRESS = 0x44;

constexpr uint8_t TEST_SAMPLE_COUNT = 10;
constexpr unsigned long SAMPLE_INTERVAL_MS = 1500;

// Second ESP32 I2C controller for the sensor test port.
TwoWire sensorBus = TwoWire(1);

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
    U8G2_R0,
    U8X8_PIN_NONE
);

// The SHT31 library will use sensorBus instead of the default Wire bus.
Adafruit_SHT31 sht31(&sensorBus);

bool oledReady = false;
bool sensorReady = false;
bool summaryPrinted = false;

uint8_t sampleNumber = 0;
uint8_t validSamples = 0;

float temperatureSum = 0.0;
float humiditySum = 0.0;

float minimumTemperature = FLT_MAX;
float maximumTemperature = -FLT_MAX;

float minimumHumidity = FLT_MAX;
float maximumHumidity = -FLT_MAX;

unsigned long lastSampleTime = 0;

bool probeAddress(TwoWire &bus, uint8_t address) {
  bus.beginTransmission(address);
  return bus.endTransmission() == 0;
}

bool isValidReading(float temperatureC, float humidityPercent) {
  if (isnan(temperatureC) || isnan(humidityPercent)) {
    return false;
  }

  if (temperatureC < -40.0 || temperatureC > 125.0) {
    return false;
  }

  if (humidityPercent < 0.0 || humidityPercent > 100.0) {
    return false;
  }

  return true;
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

void showReading(float temperatureC, float humidityPercent) {
  if (!oledReady) {
    return;
  }

  char temperatureText[24];
  char humidityText[24];
  char sampleText[24];

  snprintf(
      temperatureText,
      sizeof(temperatureText),
      "Temp: %.2f C",
      temperatureC
  );

  snprintf(
      humidityText,
      sizeof(humidityText),
      "Humidity: %.2f%%",
      humidityPercent
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

  oled.drawStr(0, 12, "SHT31 Environment");
  oled.drawStr(0, 29, temperatureText);
  oled.drawStr(0, 44, humidityText);
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
  Serial.println("SHT31 Test Summary");
  Serial.println("================================");

  Serial.print("Valid samples: ");
  Serial.print(validSamples);
  Serial.print("/");
  Serial.println(TEST_SAMPLE_COUNT);

  if (validSamples > 0) {
    float averageTemperature =
        temperatureSum / static_cast<float>(validSamples);

    float averageHumidity =
        humiditySum / static_cast<float>(validSamples);

    Serial.print("Average temperature: ");
    Serial.print(averageTemperature, 2);
    Serial.println(" C");

    Serial.print("Temperature range: ");
    Serial.print(minimumTemperature, 2);
    Serial.print(" to ");
    Serial.print(maximumTemperature, 2);
    Serial.println(" C");

    Serial.print("Average humidity: ");
    Serial.print(averageHumidity, 2);
    Serial.println(" %");

    Serial.print("Humidity range: ");
    Serial.print(minimumHumidity, 2);
    Serial.print(" to ");
    Serial.print(maximumHumidity, 2);
    Serial.println(" %");
  }

  if (validSamples == TEST_SAMPLE_COUNT) {
    Serial.println("[PASS] SHT31 produced valid environmental data");
    Serial.println("Overall result: PASS");

    showMessage(
        "SHT31 TEST",
        "RESULT: PASS",
        "10 valid samples"
    );
  } else if (validSamples > 0) {
    Serial.println("[WARN] Some samples were invalid");
    Serial.println("Overall result: PARTIAL PASS");

    showMessage(
        "SHT31 TEST",
        "PARTIAL PASS",
        "Check connection"
    );
  } else {
    Serial.println("[FAIL] No valid environmental data was received");
    Serial.println("Overall result: FAIL");

    showMessage(
        "SHT31 TEST",
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
  Serial.println("ESP32 SHT31 Environment Test");
  Serial.println("================================");

  // Start the OLED bus.
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
        "SHT31 TEST",
        "Initializing...",
        "Please wait"
    );
  } else {
    Serial.println("[WARN] OLED was not detected");
    Serial.println("The sensor test will continue without the display.");
  }

  // Start the independent SHT31 sensor bus.
  sensorBus.begin(
      SENSOR_SDA_PIN,
      SENSOR_SCL_PIN,
      100000
  );

  Serial.println("Scanning SHT31 sensor bus");

  if (!probeAddress(sensorBus, SHT31_ADDRESS)) {
    Serial.println("[FAIL] SHT31 was not detected at address 0x44");
    Serial.println("Check power, ground, SDA, and SCL.");

    showMessage(
        "SHT31 TEST",
        "RESULT: FAIL",
        "Sensor not found"
    );

    return;
  }

  Serial.println("[PASS] SHT31 detected at address 0x44");

  if (!sht31.begin(SHT31_ADDRESS)) {
    Serial.println("[FAIL] SHT31 library initialization failed");

    showMessage(
        "SHT31 TEST",
        "RESULT: FAIL",
        "Initialization"
    );

    return;
  }

  sensorReady = true;

  Serial.println("[PASS] SHT31 initialization completed");
  Serial.println();
  Serial.println("Sample,Temperature_C,Humidity_Percent");
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

  float temperatureC = sht31.readTemperature();
  float humidityPercent = sht31.readHumidity();

  sampleNumber++;

  Serial.print(sampleNumber);
  Serial.print(",");

  if (isValidReading(temperatureC, humidityPercent)) {
    Serial.print(temperatureC, 2);
    Serial.print(",");
    Serial.println(humidityPercent, 2);

    validSamples++;

    temperatureSum += temperatureC;
    humiditySum += humidityPercent;

    if (temperatureC < minimumTemperature) {
      minimumTemperature = temperatureC;
    }

    if (temperatureC > maximumTemperature) {
      maximumTemperature = temperatureC;
    }

    if (humidityPercent < minimumHumidity) {
      minimumHumidity = humidityPercent;
    }

    if (humidityPercent > maximumHumidity) {
      maximumHumidity = humidityPercent;
    }

    showReading(temperatureC, humidityPercent);
  } else {
    Serial.println("INVALID,INVALID");

    showMessage(
        "SHT31 TEST",
        "Invalid reading",
        "Check sensor"
    );
  }

  if (sampleNumber >= TEST_SAMPLE_COUNT) {
    printSummary();
  }
}