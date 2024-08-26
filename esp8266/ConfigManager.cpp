#include "ConfigManager.h"

#define EEPROM_START_ADDRESS 0

void loadConfig(Config &config)
{
  int address = EEPROM_START_ADDRESS;

  EEPROM.get(address, config.ssid);
  address += sizeof(config.ssid);

  EEPROM.get(address, config.password);
  address += sizeof(config.password);

  EEPROM.get(address, config.configMode);
  address += sizeof(config.configMode);

  EEPROM.get(address, config.name);
  address += sizeof(config.name);

  EEPROM.get(address, config.firebase_api_key);
  address += sizeof(config.firebase_api_key);

  EEPROM.get(address, config.firebase_data_base_url);
  address += sizeof(config.firebase_data_base_url);

  EEPROM.get(address, config.firebase_email);
  address += sizeof(config.firebase_email);

  EEPROM.get(address, config.firebase_password);
  address += sizeof(config.firebase_password);
}

void printConfig(const Config &config)
{
  Serial.println("-----------------------------------------------------");
  Serial.print("WiFi SSID: ");
  if (config.ssid[0] != '\0')
  {
    Serial.println(config.ssid);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("WiFi Password: ");
  if (config.password[0] != '\0')
  {
    Serial.println(config.password);
  }
  else
  {
    Serial.println("");
  }
  // Serial.print("IP: ");
  // if (WiFi.status() == WL_CONNECTED)
  // {
  //   Serial.println(WiFi.localIP());
  // }
  // else
  // {
  //   Serial.println("Não conectado");
  // }
  Serial.print("Config Mode: ");
  Serial.println(config.configMode ? "Yes" : "No");
  Serial.print("Name: ");
  if (config.name[0] != '\0')
  {
    Serial.println(config.name);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("Firebase API Key: ");
  if (config.firebase_api_key[0] != '\0')
  {
    Serial.println(config.firebase_api_key);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("Firebase DB URL: ");
  if (config.firebase_data_base_url[0] != '\0')
  {
    Serial.println(config.firebase_data_base_url);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("Firebase Email: ");
  if (config.firebase_email[0] != '\0')
  {
    Serial.println(config.firebase_email);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("Firebase Password: ");
  if (config.firebase_password[0] != '\0')
  {
    Serial.println(config.firebase_password);
  }
  else
  {
    Serial.println("");
  }
  Serial.println("-----------------------------------------------------");
}

void saveConfig(const Config &config)
{
  int address = EEPROM_START_ADDRESS;

  // Salva o SSID
  EEPROM.put(address, config.ssid);
  address += sizeof(config.ssid);

  // Salva a senha
  EEPROM.put(address, config.password);
  address += sizeof(config.password);

  // Salva o modo de configuração
  EEPROM.put(address, config.configMode);
  address += sizeof(config.configMode);

  EEPROM.put(address, config.name);
  address += sizeof(config.name);

  EEPROM.put(address, config.firebase_api_key);
  address += sizeof(config.firebase_api_key);

  EEPROM.put(address, config.firebase_data_base_url);
  address += sizeof(config.firebase_data_base_url);

  EEPROM.put(address, config.firebase_email);
  address += sizeof(config.firebase_email);

  EEPROM.put(address, config.firebase_password);
  address += sizeof(config.firebase_password);

  EEPROM.commit(); // Garante que as mudanças sejam salvas
}