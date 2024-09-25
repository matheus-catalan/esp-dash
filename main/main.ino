#include "HTTPServer.h"
#include "WifiManager.h"
#include "ConfigManager.h"
#include "ApiManager.h"
//#include "DisplayManager.h"

Config config;
ApiManager apiManager;

int sendDataPrevMillis = 0;
int count = 0;

void debugEEPROM(int address, size_t length) {
  Serial.print("EEPROM dump at address ");
  Serial.println(address);
  for (size_t i = 0; i < length; ++i) {
    Serial.print((char)EEPROM.read(address + i));
  }
  Serial.println();
}

void clearEEPROM() {
  const int EEPROM_SIZE = 512;

  // Itera sobre todos os endereços da EEPROM e escreve o valor 0xFF
  for (int i = 0; i < EEPROM_SIZE; ++i) {
    EEPROM.write(i, 0xFF); // Escreve 0xFF em cada endereço
  }
  EEPROM.commit(); // Commit para garantir que as mudanças sejam gravadas
  Serial.println("EEPROM cleared.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  setupDisplay();
  drawText("Iniciando...");
  Serial.println("Iniciando...");
  delay(500);
  drawProgressBar(10, "Iniciando Sensores...");
  initSensors();
  drawProgressBar(20, "Iniciando EEPROM...");
  EEPROM.begin(512);
  drawProgressBar(30, "Carregando configurações...");
  saveConfig(config);
  drawProgressBar(40, "Carregando configurações...");
  loadConfig(config);
  drawProgressBar(50, "Conectando Wifi...");
  connectToWiFi(config.ssid, config.password);
  delay(1000);
  drawProgressBar(60, "Iniciando http server...");
  setupNetwork();
  drawProgressBar(70, "Iniciando http server...");
  setupHttpServer();
  drawProgressBar(80, "Conectando ao Servidor...");
  apiManager.connectToServer(config);
  drawProgressBar(90, "Conectando ao MQTT...");
  apiManager.connectToMqtt(config);
  drawProgressBar(100, "Finalizando...");
  delay(1000);
  printConfig(config);
  drawHomeScreen(config.name, getWifiStatus(), getWifiSSID(), getWifiIP(), getServerStatus(), getPasswd());
}

unsigned long previous = 0;

void loop() {
  handleClient();
  apiManager.handleMqttServer(config);
  drawHomeScreen(config.name, getWifiStatus(), getWifiSSID(), getWifiIP(), getServerStatus(), getPasswd());
  readMQ2();
  checkSensorStatus(config);
  unsigned long currentMillis = millis();
  if (currentMillis - previous >= 1000) {
    //Serial.println("Sending data...");
    apiManager.sendData(config);
    //apiManager.pingToServer(config);
    previous = currentMillis;
  }
  
}