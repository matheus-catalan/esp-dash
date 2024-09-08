#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
#include <EEPROM.h>
#include <Arduino.h>
#include "SensorsManager.h"
#include "WifiManager.h"
#include "DisplayManager.h"

struct Config
{
  char ssid[32] = "Alma";
  char password[32] = "36526982";
  char name[32] = "producao";
  char base_url[64] = "http://192.168.1.23:8000/";
  char mqtt_url[14] = "192.168.1.23";
  uint16_t mqtt_port = 1884;
  char mqtt_topic[32] = "/sensors";
  bool configMode = false;
  int temperature_max = 30;
  int temperature_min = 20;
  bool temperature_alert_sound = false;
  bool temperature_alert_light = false;
  int humidity_max = 80;
  int humidity_min = 60;
  bool humidity_alert_sound = false;
  bool humidity_alert_light = false;
  int mq2_max = 1000;
  int mq2_min = 200;
  bool mq2_alert_sound = false;
  bool mq2_alert_light = false;
  int noise_max = 100;
  int noise_min = 50;
  bool noise_alert_sound = false;
  bool noise_alert_light = false;
  int ldr_max = 1000;
  int ldr_min = 200;
  bool ldr_alert_sound = false;
  bool ldr_alert_light = false;
  bool presence = true;
  bool presence_alert_sound = false;
  bool presence_alert_light = false;
  bool alert_sound = true;
  bool alert_light = true;
};

void loadConfig(Config &config);
void printConfig(const Config &config);
void saveConfig(const Config &config);
void loadConfigToFirebase(Config &config);

#endif
