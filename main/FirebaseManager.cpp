
#include "FirebaseManager.h"
#if defined(ESP8266)
  #include <FirebaseESP8266.h>
#else
  #include <FirebaseESP32.h>
#endif

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

/*FirebaseData fbdo;
FirebaseAuth firebase_auth;
FirebaseConfig firebase_config;
float _temperature = 0.0;
float _humidity = 0.0;
float _mq2 = 0.0;
bool _presence = false;
float _ldr = 0.0;
bool _noise = false;
String _wifiSSID = "";
String _ipAddress = "";
uint32_t memory_usage = 0; 
bool firstTime = true;

void updateConfigField(FirebaseJson &json, FirebaseJsonData &jsonData, const char *fieldName, int &configField, Config &config)
{
  if (json.get(jsonData, fieldName) && jsonData.type == "int" && configField != jsonData.intValue)
  {
    configField = jsonData.intValue;
    saveConfig(config);
    Serial.println("===================================");
    Serial.println("Config updated: " + String(fieldName) + " = " + String(configField));
    Serial.println("===================================");
  }
}

void updateConfigFieldBool(FirebaseJson &json, FirebaseJsonData &jsonData, const char *fieldName, bool &configField, Config &config)
{
  if (json.get(jsonData, fieldName) && jsonData.type == "boolean" && configField != jsonData.boolValue)
  {
    configField = jsonData.boolValue;
    saveConfig(config);
    Serial.println("===================================");
    Serial.println("Config updated: " + String(fieldName) + " = " + String(configField));
    Serial.println("===================================");
  }
}

void checkForConfigUpdates(Config &config)
{
  String configName = config.name;
  String path = "/enviroments/" + configName + "/config/sensors/";

  if (Firebase.getJSON(fbdo, path))
  {
    FirebaseJson &json = fbdo.jsonObject();
    FirebaseJsonData jsonData;

    updateConfigField(json, jsonData, "temperature_max", config.temperature_max, config);
    updateConfigField(json, jsonData, "temperature_min", config.temperature_min, config);
    updateConfigFieldBool(json, jsonData, "temperature_alert", config.temperature_alert, config);
    updateConfigFieldBool(json, jsonData, "temperature_light_alert", config.temperature_light_alert, config);
    updateConfigField(json, jsonData, "humidity_max", config.humidity_max, config);
    updateConfigField(json, jsonData, "humidity_min", config.humidity_min, config);
    updateConfigFieldBool(json, jsonData, "humidity_alert", config.humidity_alert, config);
    updateConfigFieldBool(json, jsonData, "humidity_light_alert", config.humidity_light_alert, config);
    updateConfigField(json, jsonData, "mq2_max", config.mq2_max, config);
    updateConfigField(json, jsonData, "mq2_min", config.mq2_min, config);
    updateConfigFieldBool(json, jsonData, "mq2_alert", config.mq2_alert, config);
    updateConfigFieldBool(json, jsonData, "mq2_light_alert", config.mq2_light_alert, config);
    updateConfigField(json, jsonData, "noise_max", config.noise_max, config);
    updateConfigField(json, jsonData, "noise_min", config.noise_min, config);
    updateConfigFieldBool(json, jsonData, "noise_alert", config.noise_alert, config);
    updateConfigFieldBool(json, jsonData, "noise_light_alert", config.noise_light_alert, config);
    updateConfigField(json, jsonData, "ldr_max", config.ldr_max, config);
    updateConfigField(json, jsonData, "ldr_min", config.ldr_min, config);
    updateConfigFieldBool(json, jsonData, "ldr_alert", config.ldr_alert, config);
    updateConfigFieldBool(json, jsonData, "ldr_light_alert", config.ldr_light_alert, config);
    updateConfigFieldBool(json, jsonData, "presence", config.presence, config);
    updateConfigFieldBool(json, jsonData, "presence_alert", config.presence_alert, config);
    updateConfigFieldBool(json, jsonData, "presence_light_alert", config.presence_light_alert, config);
  } 
  else {
     FirebaseJson json;

     json.set("temperature_max", config.temperature_max);
     json.set("temperature_min", config.temperature_min);
     json.set("temperature_alert", config.temperature_alert);
     json.set("temperature_light_alert", config.temperature_light_alert);
     json.set("humidity_max", config.humidity_max);
     json.set("humidity_min", config.humidity_min);
     json.set("humidity_alert", config.humidity_alert);
     json.set("humidity_light_alert", config.humidity_light_alert);
     json.set("mq2_max", config.mq2_max);
     json.set("mq2_min", config.mq2_min);
     json.set("mq2_alert", config.mq2_alert);
     json.set("mq2_light_alert", config.mq2_light_alert);
     json.set("noise_max", config.noise_max);
     json.set("noise_min", config.noise_min);
     json.set("noise_alert", config.noise_alert);
     json.set("noise_light_alert", config.noise_light_alert);
     json.set("ldr_max", config.ldr_max);
     json.set("ldr_min", config.ldr_min);
     json.set("ldr_alert", config.ldr_alert);
     json.set("ldr_light_alert", config.ldr_light_alert);
     json.set("presence", config.presence);
     json.set("presence_alert", config.presence_alert);
     json.set("presence_light_alert", config.presence_light_alert);
     Serial.println("Config not found, creating new one...");

     Firebase.setJSON(fbdo, path, json);
  }
}

String getCurrentTimestamp(long timezoneOffset) {
  configTime(timezoneOffset, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Falha ao obter o tempo";
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

  return String(buffer);
}

void connectToFirebase(Config &config) {
  Serial.println("=========================== FIREBASE ===========================");
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

  Serial.print("Conectando ao firebase ");
  while (Firebase.ready() == false) {
    Serial.print("."); 
    delay(500); 
  }

  Serial.println("\nConectado ao firebase ");
  Serial.println("=========================== FIREBASE ===========================");
}

void sendHistory(String path, String name, float data, bool status)
{

  FirebaseJson json;
  json.set("value", data);
  json.set("updated_at", getCurrentTimestamp(-10800));
  json.set("status", status);
  Firebase.push(fbdo, path + "/" + name, json);
}

void sendHistoryBool(String path, String name, bool data, bool status)
{

  FirebaseJson json;
  json.set("value", data);
  json.set("updated_at", getCurrentTimestamp(-10800));
  json.set("status", status);
  Firebase.push(fbdo, path + "/" + name, json);
}

void sendTemperatureValue(String path, Config &config, String path_history)
{
  float temperatureValue = readTemperature();
  if (_temperature == temperatureValue && !firstTime)
    return;

  FirebaseJson json;
  json.set("name", "Temperatura");
  json.set("status", status.temperature);
  json.set("value", temperatureValue);
  json.set("updated_at", getCurrentTimestamp(-10800));

  if (!Firebase.set(fbdo, path + "/sensors/temperature", json)) {
    Serial.println("========================");
    Serial.println("[Temperature Sensor] Error sending data to Firebase");
    Serial.println("[Temperature Sensor] Sensor read value" + String(temperatureValue));
    Serial.println("[Temperature Sensor] Error: " + fbdo.errorReason());
    Serial.println("========================");
  }

  sendHistory("/history/" + path_history, "temperature", temperatureValue, status.temperature);
  _temperature = temperatureValue;
}

void sendHumidityValue(String path, Config &config, String path_history)
{
  float humidityValue = readHumidity();

  if (_humidity == humidityValue && !firstTime)
    return;

  FirebaseJson json;
  json.set("name", "Umidade");
  json.set("status", status.humidity);
  json.set("value", humidityValue);
  json.set("updated_at", getCurrentTimestamp(-10800));

  if (!Firebase.set(fbdo, path + "/sensors/humidity", json)) {
    Serial.println("========================");
    Serial.println("[Humidity Sensor] Error sending data to Firebase");
    Serial.println("[Humidity Sensor] Sensor read value" + String(humidityValue));
    Serial.println("[Humidity Sensor] Error: " + fbdo.errorReason());
    Serial.println("========================");
  }

  sendHistory("/history/" + path_history, "humidity", humidityValue, status.humidity);
  _humidity = humidityValue;
}

void sendMQ2Value(String path, Config &config, String path_history)
{
  float mq2Value = readMQ2();

  if (_mq2 == mq2Value)
    return;

  FirebaseJson json;
  json.set("name", "Gases");
  json.set("status", status.mq2);
  json.set("value", mq2Value);
  json.set("updated_at", getCurrentTimestamp(-10800));
  Firebase.set(fbdo, path + "sensors/mq2", json);
  
  if (!Firebase.set(fbdo, path + "/sensors/mq2", json)) {
    Serial.println("========================");
    Serial.println("[MQ2 Sensor] Error sending data to Firebase");
    Serial.println("[MQ2 Sensor] Sensor read value" + String(mq2Value));
    Serial.println("[MQ2 Sensor] Error: " + fbdo.errorReason());
    Serial.println("========================");
  }

  sendHistory("/history/" + path_history, "mq2", mq2Value, status.mq2);
  _mq2 = mq2Value;
}

void sendNoiseValue(String path, Config &config, String path_history)
{
  float noiseValue = readNoise();
  
  if (_noise == noiseValue)
    return;

  FirebaseJson json;
  json.set("name", "Ruído");
  json.set("status", true);
  json.set("value", noiseValue);
  json.set("updated_at", getCurrentTimestamp(-10800));
  Firebase.set(fbdo, path + "sensors/noise", json);

  if (!Firebase.set(fbdo, path + "/sensors/noise", json)) {
    Serial.println("========================");
    Serial.println("[Noise Sensor] Error sending data to Firebase");
    Serial.println("[Noise Sensor] Sensor read value" + String(noiseValue));
    Serial.println("[Noise Sensor] Error: " + fbdo.errorReason());
    Serial.println("========================");
  }

  sendHistory("/history/" + path_history, "noise", noiseValue, true);
  _noise = noiseValue;
}

void sendPresenceValue(String path, Config &config, String path_history)
{
  bool presenceValue = readPresence();
  if (_presence == presenceValue)
    return;

  FirebaseJson json;
  json.set("name", "Presença");
  json.set("status", true);
  json.set("value", presenceValue);
  json.set("updated_at", getCurrentTimestamp(-10800));
  
  if (!Firebase.set(fbdo, path + "/sensors/presence", json)) {
    Serial.println("========================");
    Serial.println("[Presence Sensor] Error sending data to Firebase");
    Serial.println("[Presence Sensor] Sensor read value" + String(presenceValue));
    Serial.println("[Presence Sensor] Error: " + fbdo.errorReason());
    Serial.println("========================");
  }

  sendHistoryBool("/history/" + path_history, "presence", presenceValue, true);
  _presence = presenceValue;
}

void sendLDRValue(String path, Config &config, String path_history)
{
  float ldrValue = readLDR();

  if (_ldr == ldrValue)
    return;

  FirebaseJson json;
  json.set("name", "Luminosidade");
  json.set("status", status.ldr);
  json.set("value", ldrValue);
  json.set("updated_at", getCurrentTimestamp(-10800));
  
  if (!Firebase.set(fbdo, path + "/sensors/ldr", json)) {
    Serial.println("========================");
    Serial.println("[LDR Sensor] Error sending data to Firebase");
    Serial.println("[LDR Sensor] Sensor read value" + String(ldrValue));
    Serial.println("[LDR Sensor] Error: " + fbdo.errorReason());
    Serial.println("========================");
  }

  sendHistory("/history/" + path_history, "ldr", ldrValue, status.ldr);
  _ldr = ldrValue;
}

void setWifiValue(String path, Config &config)
{
  if (_wifiSSID == WiFi.SSID() && _ipAddress == WiFi.localIP().toString())
    return;

  String wifiSSID = WiFi.SSID();
  String ipAddress = WiFi.localIP().toString();
  FirebaseJson json;
  json.set("name", "Conexão WiFi");
  json.set("value", wifiSSID);
  json.set("status", WiFi.status() == WL_CONNECTED);
  json.set("updated_at", getCurrentTimestamp(-10800));
  json.set("extra", ipAddress);

  if (!Firebase.set(fbdo, path + "/sensors/wifi", json)) {
    Serial.println("========================");
    Serial.println("[WiFi Sensor] Error sending data to Firebase");
    Serial.println("[WiFi Sensor] Sensor read value" + String(wifiSSID));
    Serial.println("[WiFi Sensor] Sensor read extra value" + String(ipAddress));
    Serial.println("[WiFi Sensor] Error: " + fbdo.errorReason());
    Serial.println("========================");
  }
  
  _wifiSSID = wifiSSID;
  _ipAddress = ipAddress;
}

void sendMemoryValue(String path, Config &config){
  if (_wifiSSID == WiFi.SSID() && _ipAddress == WiFi.localIP().toString())
    return;
}

void sendData(Config &config)
{
  String configName = config.name;
  String basePath = "/enviroments/" + configName + "/";

  sendTemperatureValue(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-10800));
  sendHumidityValue(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-10800));
  sendMQ2Value(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-10800));
  sendNoiseValue(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-10800));
  sendPresenceValue(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-10800));
  sendLDRValue(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-10800));
  setWifiValue(basePath, config);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-10800));
  sendMemoryValue(basePath, config);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-10800));
  //Serial.println(String(readMemoryUsage()));
  firstTime = false;
}

String getFirebaseConnectionStatus(){
  if(Firebase.ready()){
    return "Conectado";
  } else {
    return "Desconectado";
  }
}*/
