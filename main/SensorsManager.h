#ifndef SENSORS_MANAGER_H
#define SENSORS_MANAGER_H
#include "DHT.h"
#include <MQUnifiedsensor.h>
#include "ConfigManager.h"

#if defined(ESP8266)
#define DHTPIN D1
#define VOLTAGE_PIN A1
#define MQ2PIN A0
#define LDR_PIN D2
#define PRESENCE_PIN D6
#define BUZZER_PIN D7
#define MQ2BOARD "ESP8266"
#define MQ2ADCBITRESOLUTION 10
#define MQ2RATIOCLEANAIR 60
#define MQ2VOLTAGERESOLUTION 5
#else
#define LDR_PIN 36
#define PRESENCE_PIN 12
#define MQ2PIN 38
#define DHTPIN 27
#define NOISE_PIN 32
#define LED_PIN 33
#define BUZZER_PIN 23
#define VOLTAGE_PIN 23
#define MQ2BOARD ("ESP-32")
#define MQ2ADCBITRESOLUTION (12)
#define MQ2RATIOCLEANAIR (9.83)
#define MQ2VOLTAGERESOLUTION (3.3)
#endif

#define DHTTYPE DHT11
#define MQ2TYPE ("MQ-2")

extern DHT dht;
extern MQUnifiedsensor MQ2;

struct Config;

struct sensorsStatus
{
  bool temperature = true;
  bool humidity = true;
  bool ldr = true;
  bool mq2 = true;
  bool presence = true;
  bool noise = true;
  bool wifi = true;
  bool server = true;
  bool mqtt = true;
};
extern sensorsStatus status;

void initSensors();
float readLDR();
float readTemperature();
float readHumidity();
float readMQ2();
bool readPresence();
int readNoise();
int readMemoryUsage();
int readCpuUsage();
int readVoltage();
int temperatureInternalRead();
void checkTemperature(Config &config);
void checkSensorStatus(Config &config);
String getServerStatus();

#endif
