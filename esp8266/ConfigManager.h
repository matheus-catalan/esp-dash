#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
#include <EEPROM.h>
#include <Arduino.h>
#include "SensorsManager.h"

struct Config
{
  char ssid[32] = "";
  char password[32] = "";
  char name[32] = "";
  char firebase_api_key[64] = "";
  char firebase_data_base_url[128] = "";
  char firebase_email[64] = "";
  char firebase_password[64] = "";
  bool configMode = false;
};

void loadConfig(Config &config);
void printConfig(const Config &config);
void saveConfig(const Config &config);

#endif
