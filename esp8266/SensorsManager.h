#ifndef SENSORS_MANAGER_H
#define SENSORS_MANAGER_H
#include "DHT.h"
#include <MQUnifiedsensor.h>

#define DHTPIN D1
#define DHTTYPE DHT11

#define MQ2BOARD "ESP8266"
#define MQ2PIN A0
#define MQ2TYPE "MQ-2"
#define MQ2VOLTAGERESOLUTION 3.3
#define MQ2ADCBITRESOLUTION 10
#define MQ2RATIOCLEANAIR 60

#define PRESENCEPIN D6


extern DHT dht;
extern MQUnifiedsensor MQ2;

struct sensorsStatus
{
  bool temperature;
  bool humidity;
  bool ldr;
  bool mq2;
};
extern sensorsStatus status;

void initSensors();
float readLDR();
float readTemperature();
float readHumidity();
float readMQ2();
bool readPresence();

#endif
