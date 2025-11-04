# ESP32-C6 Zigbee Temperature & Humidity Sensor

This project implements a **Zigbee temperature and humidity sensor** using an **ESP32-C6** with a **SHT40 sensor** and an **SH1106 OLED display**. It connects to **Zigbee2MQTT** to expose sensor readings to your smart home system.

---

## Features

- **Temperature Measurement**: Uses SHT40 sensor to measure temperature in °C.  
- **Humidity Measurement**: Reads relative humidity from the SHT40.  
- **Battery Monitoring**: Reports battery status over Zigbee.  
- **OLED Display**: Shows real-time temperature, humidity, and status on an SH1106 display.  
- **Zigbee2MQTT Integration**: Automatically reports temperature, humidity, and battery to your Zigbee2MQTT network.  
- **Factory Reset Button**: Hold button for 3 seconds to reset Zigbee configuration.  

---

## Hardware

- **Microcontroller**: ESP32-C6 DevKitC-1  
- **Temperature & Humidity Sensor**: SHT40 (I2C)  
- **Display**: SH1106 OLED (I2C, 128×64)  
- **Button**: Optional factory reset button  

---

## Wiring

| Component | ESP32-C6 Pin |
|-----------|--------------|
| SHT40 SDA | 6            |
| SHT40 SCL | 7            |
| SH1106 SDA| 6            |
| SH1106 SCL| 7            |
| Button    | 4            |

> Note: SDA and SCL are shared for I2C devices.  

---

## PlatformIO Setup

1. Clone or download the project.  
2. Install PlatformIO in VS Code.  
3. Connect ESP32-C6 board.  
4. Ensure `platformio.ini` includes:

```ini
[env:esp32-c6-devkitc-1]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip
board = esp32-c6-devkitc-1
framework = arduino
monitor_speed = 115200

build_flags = -DZIGBEE_MODE_ED

lib_deps =
    adafruit/Adafruit SH110X
    adafruit/Adafruit SHT4x Library
    espressif/Zigbee

board_build.partitions = partitions/zigbee.csv
```

5. Compile and upload firmware via PlatformIO.

---

## Zigbee2MQTT Integration

After connecting to the Zigbee network, the following **entities** are exposed:

| Entity      | Description                     | Example Value |
|-------------|---------------------------------|---------------|
| Temperature | Measured temperature (°C)       | 23.98°C       |
| Humidity    | Measured relative humidity (%)  | 55.43%        |
| Battery     | Battery percentage              | 100%          |

---

## Usage

1. Power on the ESP32-C6 with the SHT40 and SH1106 connected.  
2. The OLED will display current temperature and humidity.  
3. Button functionality:
   - Press briefly: manual sensor report.  
   - Hold 3 seconds: factory reset Zigbee network settings.  