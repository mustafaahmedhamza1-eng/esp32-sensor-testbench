# ESP32 Sensor Testbench

A hardware-tested ESP32 testbench for evaluating biomedical and environmental sensor modules individually.

The repository contains independent diagnostic programs for an OLED display, MAX30102 optical sensor, DS18B20 temperature sensor, and SHT31 temperature and humidity sensor. 
Each program checks hardware detection, sensor initialization, data acquisition, and basic failure handling.

The project is intended for learning, hardware verification, and embedded-system development. It is not a medical diagnostic device.

## Project Objectives

This repository was created to:

- verify that each sensor communicates correctly with an ESP32;
- collect and display raw sensor readings;
- detect disconnected or incorrectly wired components;
- document successful and failed hardware tests;
- provide reusable diagnostic firmware for future embedded projects.

## Hardware Used

- ESP32 development board
- SH1106 128 × 64 OLED display
- MAX30102 optical pulse and oxygen-sensing module
- DS18B20 digital temperature sensor
- SHT31 temperature and humidity sensor
- USB data cable
- Jumper wires
- 4.7 kΩ pull-up resistor for the DS18B20 when required

## Tested Firmware

| Test | Purpose | Positive Test | Disconnection Test |
|---|---|---:|---:|
| Serial connection | Verifies ESP32 programming and serial communication | Pass | Not applicable |
| OLED display | Tests I2C detection and visual display patterns | Pass | Pass |
| MAX30102 | Reads raw RED and IR optical values | Pass | Pass |
| DS18B20 | Reads digital temperature measurements | Pass | Pass |
| SHT31 | Reads temperature and relative humidity | Pass | Pass |

A successful disconnection test means that the firmware correctly detected the missing component and reported an error instead of returning misleading data.

## Repository Structure

```text
ESP32 Sensor Testbench/
├── docs/
│   └── troubleshooting.md
├── firmware/
│   ├── 00_serial_connection_test/
│   │   └── 00_serial_connection_test.ino
│   ├── 01_oled_display_test/
│   │   └── 01_oled_display_test.ino
│   ├── 02_max30102_raw_signal_test/
│   │   └── 02_max30102_raw_signal_test.ino
│   ├── 03_ds18b20_temperature_test/
│   │   └── 03_ds18b20_temperature_test.ino
│   └── 04_sht31_environment_test/
│       └── 04_sht31_environment_test.ino
├── images/
│   ├── esp32/
│   ├── oled/
│   ├── max30102/
│   ├── ds18b20/
│   └── sht31/
├── results/
│   ├── serial/
│   │   └── serial_connection_test_pass.txt
│   ├── oled/
│   │   ├── oled_display_test_pass.txt
│   │   └── oled_display_disconnected.txt
│   ├── max30102/
│   │   ├── max30102_raw_signal_test_pass.txt
│   │   └── max30102_disconnected_test.txt
│   ├── ds18b20/
│   │   ├── ds18b20_temperature_test_pass.txt
│   │   └── ds18b20_disconnected_test.txt
│   └── sht31/
│       ├── sht31_environment_test_pass.txt
│       └── sht31_disconnected_test.txt
├── .gitignore
├── LICENSE
└── README.md
```

## Example GPIO Configuration

The published sketches use a configurable example testbench layout.

| Component | Signal | ESP32 GPIO |
|---|---|---:|
| OLED | SDA | 21 |
| OLED | SCL | 22 |
| MAX30102 | SDA | 32 |
| MAX30102 | SCL | 33 |
| SHT31 | SDA | 32 |
| SHT31 | SCL | 33 |
| DS18B20 | DATA | 32 |

Only one sensor should be connected to the shared sensor test port at a time.

The firmware logic was validated using physical sensor modules. 
Some local hardware tests used an existing prototype wiring arrangement, while the published GPIO assignments provide a general and reusable testbench configuration.

## Documentation

More information is available in:

- [Troubleshooting](docs/troubleshooting.md)

## Software Environment

The programs were developed and tested using:

```text
Arduino IDE: 2.3.7
Board: ESP32 Dev Module
ESP32 board package: esp32 by Espressif Systems 3.3.4
Serial baud rate: 115200
USB interface: Silicon Labs CP210x
```

## Required Arduino Libraries

The sketches use the following libraries:

- U8g2
- OneWire
- DallasTemperature
- SparkFun MAX3010x Pulse and Proximity Sensor Library
- Adafruit SHT31 Library
- Adafruit BusIO

Install the libraries through the Arduino IDE Library Manager before compiling the corresponding sketches.

## Running a Test

1. Disconnect the ESP32 from USB power.
2. Connect the required component according to the wiring documentation.
3. Open the corresponding .ino file in Arduino IDE.
4. Select ESP32 Dev Module.
5. Select the correct COM port.
6. Compile and upload the program.
7. Open Serial Monitor at 115200 baud.
8. Press the ESP32 RST button.
9. Save the resulting output inside the appropriate results folder.

Power should always be disconnected before changing sensor wiring.

## MAX30102 Test Result

The MAX30102 test successfully detected the sensor at I2C address 0x57 and collected 20 valid RED and IR samples.

Example result:

```text
Valid samples: 20/20
RED range: 63585 to 65497
IR range: 54075 to 54531
Overall result: PASS
```

These values represent raw optical measurements only. The test does not validate heart-rate or blood-oxygen accuracy.

## SHT31 Test

The SHT31 test records ten temperature and relative-humidity measurements and reports:

- valid sample count;
- average temperature;
- minimum and maximum temperature;
- average humidity;
- minimum and maximum humidity.

The test verifies communication and short-term reading stability. It does not represent a calibration or accuracy comparison against a reference instrument.

## Failure Detection

Each sensor program includes a negative test.

Examples of detected failures include:

```text
[FAIL] MAX30102 was not detected at address 0x57
[FAIL] DS18B20 was not detected
[FAIL] SHT31 was not detected at address 0x44
```

These tests demonstrate that the firmware can identify disconnected components and provide useful troubleshooting messages.

## Hardware Test Images

Photographs of the physical modules and OLED test results are available in the [images directory](images).

The ESP32 controller was integrated into an existing prototype during some tests. 
Therefore, the images focus on the tested modules and displayed results without exposing the internal prototype assembly.

## Limitations

- This repository is a component testbench, not a complete patient-monitoring system.
- The sensors are tested individually.
- MAX30102 results are raw optical signals and are not clinical SpO2 or heart-rate measurements.
- Temperature and humidity tests confirm communication and basic stability, not laboratory calibration.
- Results may vary according to wiring quality, power supply, sensor module design, and environmental conditions.


## Future Development

Possible future additions include:

- longer-duration stability tests;
- CSV data logging;
- graphical analysis of sensor readings;
- timing and sampling-rate measurements;
- automated sensor recovery after communication failure;
- comparison with calibrated reference instruments.

Wireless communication between multiple ESP32 nodes will be developed as a separate repository.

## Author

**Mustafa Ahmed Al-Kazali**

BSc in Medical Instrumentation Techniques Engineering

## License

See the [LICENSE file](LICENSE) for the terms of use.
