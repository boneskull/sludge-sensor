#include <Arduino.h>
#include <ConfigManager.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
// #include <ArduinoOTA.h>

#define READING_MS 30000
#define WARMUP_MS 10000
#define NUM_SAMPLES 30
#define SAMPLING_MS 250
#define UPDATE_MS 250
#define AIR_VALUE 520
#define WATER_VALUE 260

struct Config {
  char hostname[24];
  bool enabled;
  uint8_t read_interval_ms;
  uint8_t max_value;
  uint8_t min_value;
} config;

ConfigManager configManager;

const float intervals = (AIR_VALUE - WATER_VALUE) / 4;
const int green_pin = 15;
const int yellow_pin = 14;
const int red_pin = 13;
const int sensor_pin = A0;

enum SensorState { idle, sampling };
enum IndicatorState {
  off,
  too_wet_on,
  too_wet_off,
  ok,
  warning,
  alert_on,
  alert_off
};
IndicatorState indicatorState = off;
SensorState sensorState = idle;

unsigned long prevUpdateMs = 0;
unsigned long prevSamplingMs = 0;
unsigned long prevReadingMs = 0;
int sample = 0;
int samples[NUM_SAMPLES];

/** Setup method. */
void setup() {
  Serial.begin(115200);
  Serial.println("Serial initialized");

  pinMode(green_pin, OUTPUT);
  pinMode(yellow_pin, OUTPUT);
  pinMode(red_pin, OUTPUT);
  digitalWrite(green_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(red_pin, LOW);

  configManager.setAPName("Sludge Sensor");
  configManager.setAPFilename("/index.html");
  // configManager.addParameter("Hostname", config.hostname, 24);
  // configManager.addParameter("Reading Interval (ms)",
  // &config.read_interval_ms);
  // configManager.addParameter("Max Sensor Value", &config.max_value);
  // configManager.addParameter("Min Sensor Value", &config.min_value);
  Serial.println("Initializing Config Manager");
  configManager.begin(config);
  Serial.println("Config Manager initialized");

  // if (config.hostname) {
  //   Serial.println("Set hostname to " + String(config.hostname));
  //   WiFi.hostname(String(config.hostname));
  // }
  //
  // ArduinoOTA.onStart([]() {
  //   Serial.println("Start");
  // });
  // ArduinoOTA.onEnd([]() {
  //   Serial.println("\nEnd");
  // });
  // ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  // });
  // ArduinoOTA.onError([](ota_error_t error) {
  //   Serial.printf("Error[%u]: ", error);
  //   if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
  //   else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
  //   else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
  //   else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
  //   else if (error == OTA_END_ERROR) Serial.println("End Failed");
  // });
  // ArduinoOTA.begin();

  Serial.println("Setup complete");
}

void reset() {
  digitalWrite(green_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(red_pin, LOW);
}

void indicateState() {
  switch (indicatorState) {
  case ok:
    digitalWrite(green_pin, HIGH);
    break;
  case too_wet_off:
    digitalWrite(green_pin, HIGH);
    indicatorState = too_wet_on;
    break;
  case too_wet_on:
    digitalWrite(green_pin, LOW);
    indicatorState = too_wet_off;
    break;
  case warning:
    digitalWrite(yellow_pin, HIGH);
    break;
  case alert_on:
    digitalWrite(red_pin, LOW);
    indicatorState = alert_off;
    break;
  case alert_off:
    digitalWrite(red_pin, HIGH);
    indicatorState = alert_on;
    break;
  case off:
    reset();
    break;
  }
}

void setIndicatorState(float moistureValue) {
  if (moistureValue >= AIR_VALUE - intervals) {
    indicatorState = alert_off;
  } else if (moistureValue >= AIR_VALUE - (2 * intervals)) {
    indicatorState = warning;
  } else if (moistureValue >= AIR_VALUE - (3 * intervals)) {
    indicatorState = ok;
  } else {
    indicatorState = too_wet_off;
  }
  reset();
}

/** Reads samples from the sensor.
 *
 *  @param samples The array where to store the samples.
 */
void readSensor() {
  samples[sample] = analogRead(sensor_pin) * 3;
  Serial.print(F("Sensor value: "));
  Serial.println(samples[sample]);
}

/** Loop method. */
void loop() {
  // ArduinoOTA.handle();
  configManager.loop();

  if (sensorState == sampling) {
    reset();
    digitalWrite(red_pin + (sample % 3), HIGH);
  }
  unsigned long currentMillis = millis();
  if (currentMillis >= prevUpdateMs + UPDATE_MS) {
    indicateState();
    prevUpdateMs = currentMillis;
  }
  switch (sensorState) {
  case sampling:
    if (currentMillis >= prevSamplingMs + SAMPLING_MS) {
      if (sample == NUM_SAMPLES - 1) {
        float avg = 0.0;
        int sum = 0;
        for (int i = 0; i < NUM_SAMPLES; i++) {
          sum += samples[i];
        }
        avg = (float(sum)) / NUM_SAMPLES;
        Serial.print("Mean: ");
        Serial.println(avg);
        sample = 0;
        setIndicatorState(avg);
        sensorState = idle;
      } else {
        readSensor();
        sample++;
        prevSamplingMs = currentMillis;
      }
    }
    break;
  case idle:
    if (currentMillis >= prevReadingMs + READING_MS) {
      sensorState = sampling;
      prevSamplingMs = currentMillis - SAMPLING_MS;
      prevReadingMs = currentMillis;
    }
    break;
  }
}
