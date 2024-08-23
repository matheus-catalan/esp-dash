#include "FirebaseManager.h"
#include <FirebaseESP8266.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

FirebaseData fbdo;
FirebaseAuth firebase_auth;
FirebaseConfig firebase_config;

String getCurrentTimestamp()
{
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}

void connectToFirebase(Config &config)
{
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  firebase_config.api_key = config.firebase_api_key;
  firebase_auth.user.email = config.firebase_email;
  firebase_auth.user.password = config.firebase_password;
  firebase_config.database_url = config.firebase_data_base_url;
  firebase_config.token_status_callback = tokenStatusCallback;

  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.reconnectNetwork(true);
  Firebase.begin(&firebase_config, &firebase_auth);
  Firebase.setDoubleDigits(5);
}

void sendData(Config &config)
{
  String configName = config.name; // Substitua pelo nome da configuração real
  String basePath = "/enviroments/" + configName + "/current/";
  String historyPath = "/enviroments/" + configName + "/history/";

  // Ler os valores dos sensores
  float dhtValue = readDHT11();                // Leitura da temperatura do DHT11
  float mq2Value = readMQ2();                  // Leitura do MQ2
  float lumiValue = random(200, 1000) / 10.0;  // Exemplo: valores entre 20.0 e 100.0
  float noiseValue = random(500, 1500) / 10.0; // Exemplo: valores entre 50.0 e 150.0
  float ldrValue = readLDR();
  String ipAddress = WiFi.localIP().toString();
  String wifiSSID = WiFi.SSID();
  String lastUpdate = getCurrentTimestamp();

  // Atualizar os valores mais recentes nos campos específicos
  Serial.printf("Set dht... %s\n", Firebase.setFloat(fbdo, basePath + "dht", dhtValue) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set mq2... %s\n", Firebase.setFloat(fbdo, basePath + "mq2", mq2Value) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set lumi... %s\n", Firebase.setFloat(fbdo, basePath + "lumi", lumiValue) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set noise... %s\n", Firebase.setFloat(fbdo, basePath + "noise", noiseValue) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set ldr... %s\n", Firebase.setFloat(fbdo, basePath + "ldr", ldrValue) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set ip... %s\n", Firebase.setString(fbdo, basePath + "ip", ipAddress) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set wifi... %s\n", Firebase.setString(fbdo, basePath + "wifi", wifiSSID) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set last_update... %s\n", Firebase.setString(fbdo, basePath + "last_update", lastUpdate) ? "ok" : fbdo.errorReason().c_str());

  // Construir o JSON com os valores atuais e timestamp
  FirebaseJson json;
  json.set("dht", dhtValue);
  json.set("mq2", mq2Value);
  json.set("lumi", lumiValue);
  json.set("noise", noiseValue);
  json.set("ldr", ldrValue);
  json.set("ip", ipAddress);
  json.set("wifi", wifiSSID);
  json.set("last_update", lastUpdate);

  // Adicionar o novo registro ao histórico
  Serial.printf("Append to history... %s\n", Firebase.push(fbdo, historyPath, json) ? "ok" : fbdo.errorReason().c_str());

  Serial.println();
}
