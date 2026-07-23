# Troubleshooting Notes

This file summarizes the main problems I encountered while preparing and testing the ESP32 sensor programs in this repository.

## ESP32 Was Not Detected by Arduino IDE

At the beginning, Arduino IDE did not show a usable COM port, so I could not upload programs to either ESP32 board.

The boards used in this project have a Silicon Labs CP210x USB-to-UART interface. Installing the **Silicon Labs CP210x Universal Windows Driver** solved the problem.

After installing the driver, Windows Device Manager displayed the board as:

```text
Silicon Labs CP210x USB to UART Bridge
```

The board then appeared in Arduino IDE and could be programmed normally.

The settings used during development were:

```text
Board: ESP32 Dev Module
ESP32 package: esp32 by Espressif Systems 3.3.4
Arduino IDE: 2.3.7
```

The port was `COM3` during testing, but the COM number may change when another USB port or ESP32 board is used.

I also downloaded the CH340/CH341 driver while investigating the issue, but I did not install it because the CP210x driver had already solved the problem.
## Checking the USB Connection

A powered ESP32 does not always mean that the USB connection is working correctly. Some USB cables provide power but do not support data transfer.

When the board is not detected, it is useful to check:

- that the cable supports data transfer;
- that the correct USB driver is installed;
- that the correct COM port is selected;
- that `ESP32 Dev Module` is selected in Arduino IDE.

## SHT31 Library Compatibility

An early version of the SHT31 test did not compile because the installed library used a different initialization format.

I updated the code to match the installed library version. After that, the sketch compiled successfully and the sensor operated correctly.