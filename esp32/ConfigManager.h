#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
#include <EEPROM.h>
#include <Arduino.h>


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
  int temperature_max = 30;
  int temperature_min = 20;
  bool temperature_alert = false;
  bool temperature_light_alert = false;
  int humidity_max = 80;
  int humidity_min = 60;
  bool humidity_alert = false;
  bool humidity_light_alert = false;
  int mq2_max = 1000;
  int mq2_min = 200;
  bool mq2_alert = false;
  bool mq2_light_alert = false;
  int noise_max = 100;
  int noise_min = 50;
  bool noise_alert = false;
  bool noise_light_alert = false;
  int ldr_max = 1000;
  int ldr_min = 200;
  bool ldr_alert = false;
  bool ldr_light_alert = false;
  bool presence = true;
  bool presence_alert = false;
  bool presence_light_alert = false;

  bool sound_alert = true;
};

void loadConfig(Config &config);
void printConfig(const Config &config);
void saveConfig(const Config &config);
void loadConfigToFirebase(Config &config);

#endif
