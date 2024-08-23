#ifndef SENSORS_MANAGER_H
#define SENSORS_MANAGER_H
#include "DHT.h"

#define DHTPIN D1
#define DHTTYPE DHT11
extern DHT dht;

float readLDR();
float readDHT11();
float readMQ2();

#endif
