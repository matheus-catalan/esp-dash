#include "ConfigManager.h"

#define EEPROM_START_ADDRESS 0


#include <EEPROM.h>

void loadCharField(char* buffer, size_t fieldSize, int& address) {
  bool isEmpty = true; // Flag para verificar se a EEPROM está vazia
  
  // Lê os dados da EEPROM para o buffer
  for (size_t i = 0; i < fieldSize - 1; ++i) {
    buffer[i] = EEPROM.read(address + i);
  }
  buffer[fieldSize - 1] = '\0'; // Garante que o buffer seja nulo-terminado
  address += fieldSize;


  EEPROM.commit(); // Comita para garantir que as alterações sejam salvas
  Serial.print(".");
  delay(100);

}

void printConfig(const Config &config)
{ 
  Serial.println("WiFi SSID             -> " + String(config.ssid));
  Serial.println("WiFi Password         -> " + String(config.password));
  Serial.println("Name                  -> " + String(config.name));
  Serial.println("URL SERVIDOR          -> " + String(config.base_url));
  Serial.println("Temperature           -> Alerta sonoro: " + String(config.temperature_alert ? "Yes" : "No ") + " | Temperatura Min: " + String(config.temperature_min) + " | Temperatura Max: " + String(config.temperature_max));
  Serial.println("Humidade              -> Alerta sonoro: " + String(config.humidity_alert ? "Yes" : "No ") + " | Humidade Min: " + String(config.humidity_min) + "       | Humidade Max: " + String(config.humidity_max));
  Serial.println("MQ2                   -> Alerta sonoro: " + String(config.mq2_alert ? "Yes" : "No ") + " | MQ2 Min: " + String(config.mq2_min) + "                 | MQ2 Max: " + String(config.mq2_max));
  Serial.println("Noise                 -> Alerta sonoro: " + String(config.noise_alert ? "Yes" : "No ") + " | Noise Min: " + String(config.noise_min) + "             | Noise Max: " + String(config.noise_max));
  Serial.println("LDR                   -> Alerta sonoro: " + String(config.ldr_alert ? "Yes" : "No ") + " | LDR Min: " + String(config.ldr_min) + "                 | LDR Max: " + String(config.ldr_max));
  Serial.println("Presence -> Alerta sonoro: " + String(config.presence_alert ? "Yes" : "No")); 
}

void loadConfig(Config &config) {
  int address = EEPROM_START_ADDRESS;
  Serial.println("=========================== CONFIGURAÇÕES ===========================");
  Serial.print("Carregando configurações...");
  loadCharField(config.ssid, sizeof(config.ssid), address);
  loadCharField(config.password, sizeof(config.password), address);
  loadCharField(config.name, sizeof(config.name), address);
  loadCharField(config.base_url, sizeof(config.base_url), address);
  Serial.println("\nConfigurações carregadas");
  Serial.println("=========================== CONFIGURAÇÕES ===========================");
}

void saveStringToEEPROM(int address, const char* str) {
  int len = strlen(str); // Obtém o comprimento da string
  if (len >= 32) { // Verifica se o comprimento excede o tamanho do buffer
    len = 31; // Ajusta o comprimento se necessário
  }
  
  // Grava a string na EEPROM e adiciona um caractere nulo ao final
  for (int i = 0; i < len; i++) {
    EEPROM.write(address + i, str[i]);
  }
  EEPROM.write(address + len, '\0'); // Adiciona o caractere nulo no final

  EEPROM.commit(); // Garante que as alterações sejam salvas
}

void saveStringToEEPROM(const char* str, int& address, size_t size) {
  for (size_t i = 0; i < size - 1; ++i) {
    EEPROM.write(address + i, str[i]);
  }
  EEPROM.write(address + size - 1, '\0'); 
  address += size; // Atualiza o endereço após salvar
}

void saveConfig(const Config &config) {
  int address = EEPROM_START_ADDRESS;

  // Salva o SSID
  saveStringToEEPROM(config.ssid, address, sizeof(config.ssid));

  // Salva a senha
  saveStringToEEPROM(config.password, address, sizeof(config.password));

  // Salva o nome
  saveStringToEEPROM(config.name, address, sizeof(config.name));

  // Salva a chave da API do Firebase
  saveStringToEEPROM(config.base_url, address, sizeof(config.base_url));

  EEPROM.commit();
}
