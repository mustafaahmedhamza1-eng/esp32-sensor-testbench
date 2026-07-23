#include <Wire.h>
#include <U8g2lib.h>

constexpr uint8_t OLED_SDA_PIN = 21;
constexpr uint8_t OLED_SCL_PIN = 22;

constexpr uint8_t OLED_ADDRESS_PRIMARY = 0x3C;
constexpr uint8_t OLED_ADDRESS_SECONDARY = 0x3D;

constexpr unsigned long PAGE_INTERVAL_MS = 2500;

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
    U8G2_R0,
    U8X8_PIN_NONE
);

uint8_t detectedAddress = 0;
uint8_t currentPage = 0;
unsigned long lastPageChange = 0;

bool probeI2CAddress(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

uint8_t detectOLEDAddress() {
  if (probeI2CAddress(OLED_ADDRESS_PRIMARY)) {
    return OLED_ADDRESS_PRIMARY;
  }

  if (probeI2CAddress(OLED_ADDRESS_SECONDARY)) {
    return OLED_ADDRESS_SECONDARY;
  }

  return 0;
}

void showIdentificationPage() {
  char addressText[24];

  snprintf(
      addressText,
      sizeof(addressText),
      "I2C address: 0x%02X",
      detectedAddress
  );

  oled.clearBuffer();
  oled.setFont(u8g2_font_6x12_tr);

  oled.drawStr(0, 13, "ESP32 OLED Test");
  oled.drawStr(0, 30, "Controller: SH1106");
  oled.drawStr(0, 45, "Resolution: 128x64");
  oled.drawStr(0, 60, addressText);

  oled.sendBuffer();
}

void showGeometryPage() {
  oled.clearBuffer();

  oled.drawFrame(0, 0, 128, 64);
  oled.drawLine(0, 0, 127, 63);
  oled.drawLine(127, 0, 0, 63);

  oled.drawCircle(32, 32, 14);
  oled.drawDisc(96, 32, 12);

  oled.sendBuffer();
}

void showCheckerboardPage() {
  oled.clearBuffer();

  constexpr uint8_t blockSize = 8;

  for (uint8_t y = 0; y < 64; y += blockSize) {
    for (uint8_t x = 0; x < 128; x += blockSize) {
      if (((x / blockSize) + (y / blockSize)) % 2 == 0) {
        oled.drawBox(x, y, blockSize, blockSize);
      }
    }
  }

  oled.sendBuffer();
}

void showAllPixelsPage() {
  oled.clearBuffer();
  oled.drawBox(0, 0, 128, 64);

  oled.setDrawColor(0);
  oled.setFont(u8g2_font_6x12_tr);
  oled.drawStr(18, 36, "ALL PIXELS ON");
  oled.setDrawColor(1);

  oled.sendBuffer();
}

void displayCurrentPage() {
  switch (currentPage) {
    case 0:
      showIdentificationPage();
      Serial.println("Page 1: identification");
      break;

    case 1:
      showGeometryPage();
      Serial.println("Page 2: geometry");
      break;

    case 2:
      showCheckerboardPage();
      Serial.println("Page 3: checkerboard");
      break;

    case 3:
      showAllPixelsPage();
      Serial.println("Page 4: all pixels");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 OLED Display Test");
  Serial.println("================================");

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN, 100000);

  detectedAddress = detectOLEDAddress();

  if (detectedAddress == 0) {
    Serial.println("[FAIL] OLED display was not detected");
    Serial.println("Check power, ground, SDA, and SCL.");
    return;
  }

  Serial.print("[PASS] OLED detected at address 0x");
  Serial.println(detectedAddress, HEX);

  oled.setI2CAddress(detectedAddress << 1);
  oled.begin();

  Serial.println("[PASS] OLED library initialization completed");
  Serial.println("[INFO] Four visual test pages will repeat");
  Serial.println();

  displayCurrentPage();
  lastPageChange = millis();
}

void loop() {
  if (detectedAddress == 0) {
    return;
  }

  unsigned long now = millis();

  if (now - lastPageChange >= PAGE_INTERVAL_MS) {
    lastPageChange = now;

    currentPage++;
    if (currentPage >= 4) {
      currentPage = 0;
    }

    displayCurrentPage();
  }
}