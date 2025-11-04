/**
 * Proper Zigbee mode must be selected in Tools->Zigbee mode
 * and also the correct partition scheme must be selected in Tools->Partition Scheme.
 */

//#include <Arduino.h>
#include <Wire.h>
#include <Zigbee.h>
#include <Adafruit_SHT4x.h>
#include <Adafruit_SH110X.h>
#include <stdarg.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

/* Zigbee temperature sensor configuration */
#define TEMP_SENSOR_ENDPOINT_NUMBER 10
const int button = 4; // or use BOOT_PIN

// SHT40 and Display initialization
Adafruit_SHT4x sht4 = Adafruit_SHT4x();
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

sensors_event_t humidity, temperature;

// Optional Time cluster variables
struct tm timeinfo;
struct tm *localTime;
int32_t timezone;

ZigbeeTempSensor zbTempSensor = ZigbeeTempSensor(TEMP_SENSOR_ENDPOINT_NUMBER);

/********************** Print functions ***************************/
void print_to_display_and_terminal(const char* format, ...) {
  char buffer[128];

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Serial.println(buffer);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(buffer);
  display.display();
}

/************************ Temp sensor *****************************/
void temp_sensor_value_update() {
  // Read temperature sensor value from SHT40
  sht4.getEvent(&humidity, &temperature);

  // Update temperature and humidity values in Temperature sensor EP
  zbTempSensor.setTemperature(temperature.temperature);
  zbTempSensor.setHumidity(humidity.relative_humidity);
  
  // Send values to zigbee coodinator
  zbTempSensor.report();

  // Print new values
  print_to_display_and_terminal("Temperature: %.2f%cC \nHumidity: %.2f%% rH   ", temperature.temperature, 247, humidity.relative_humidity);
  
  // Delay
  delay(1000);
}

/********************* Arduino functions **************************/
void setup() {
  Serial.begin(115200);

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  // Initialize I2C for display
  Wire.begin(6, 7);  // SDA, SCL for ESP32-C6

  // Initialize OLED display
  if (!display.begin(0x3C, true)) {
    print_to_display_and_terminal("SH1106 not found");
    for (;;);
  }

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Initialize SHT40 sensor
  if (!sht4.begin()) {
    print_to_display_and_terminal("SHT40 not found");
    for (;;);
  }

  sht4.setPrecision(SHT4X_LOW_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);  

  // Init button switch
  pinMode(button, INPUT_PULLUP);

  // Optional: set Zigbee device name and model
  zbTempSensor.setManufacturerAndModel("Espressif", "ZigbeeTempHumSensor");

  // Set minimum and maximum temperature measurement value (adjusted for DHT11 range: 0-50°C)
  zbTempSensor.setMinMaxValue(0, 50);

  // Optional: Set tolerance for temperature measurement in °C
  zbTempSensor.setTolerance(1);

  // Add humidity cluster to the temperature sensor device with min, max and tolerance values
  zbTempSensor.addHumiditySensor(0, 100, 1);

  // Optional: Time cluster configuration (default params, as this device will receive time from coordinator)
  zbTempSensor.addTimeCluster();

  // Add endpoint to Zigbee Core
  Zigbee.addEndpoint(&zbTempSensor);

  // Set power source to battery, battery percentage and battery voltage (now 100% and 3.5V for demonstration)
  // The value can be also updated by calling zbTempSensor.setBatteryPercentage(percentage) or zbTempSensor.setBatteryVoltage(voltage) anytime after Zigbee.begin()
  zbTempSensor.setPowerSource(ZB_POWER_SOURCE_BATTERY, 100, 35);

  
  print_to_display_and_terminal("Starting Zigbee...");
  // When all EPs are registered, start Zigbee in End Device mode
  if (!Zigbee.begin()) {
    print_to_display_and_terminal("Zigbee failed!\nRebooting...");
    delay(2000);
    ESP.restart();
  } else {
    print_to_display_and_terminal("Zigbee started successfully!");
  }
  
  print_to_display_and_terminal("Connecting to network");
  
  while (!Zigbee.connected()) {
    delay(100);
  }

  print_to_display_and_terminal("Connected!");

  delay(1000);

  // Get time from Zigbee
  timeinfo = zbTempSensor.getTime();
  timezone = zbTempSensor.getTimezone();

  // Convert to local time
  time_t local = mktime(&timeinfo) + timezone;
  localTime = localtime(&local);

  // Format local time to string
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", localTime);

  // Print to Serial and OLED using your function
  print_to_display_and_terminal("Local time:\n%s", timeStr);

  // Start Temperature sensor reading task
  //xTaskCreate(temp_sensor_value_update, "temp_sensor_update", 2048, NULL, 10, NULL);

  // Set reporting interval for temperature and humidity measurement in seconds
  // min_interval and max_interval in seconds, delta (temp change in 0.1 °C)
  // min = 0, max = 60 means: report every 60 seconds OR when value changes by delta
  //zbTempSensor.setReporting(0, 60, 5);  // Report every 60s or on 0.5°C change
  
  //Serial.println("Automatic reporting configured: every 60s or on change");
}

void loop() {
  // Checking button for factory reset
  if (digitalRead(button) == LOW) {  // Push button pressed
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(button) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        // If key pressed for more than 3secs, factory reset Zigbee and reboot
        print_to_display_and_terminal("Factory Reset...");
        delay(100);
        Zigbee.factoryReset();
      }
    }
    // Manual report of temperature and humidity
    temp_sensor_value_update();
  }
  delay(100);
}