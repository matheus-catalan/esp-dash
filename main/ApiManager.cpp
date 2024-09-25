#include "ApiManager.h"
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


ApiManager::ApiManager() {}
WiFiClient espClient;
PubSubClient mqttClient(espClient);

float _temperature = 0.0;
float _humidity = 0.0;
float _mq2 = 0.0;
int _noise = 0; 
int _ldr = 0.0;
int _memory = 0.0;
int _cpu = 0.0;
int _voltage = 0.0;
int _internal_temperature = 0.0;
bool _presence = false;
unsigned long _uptime = 0;
String _wifiSSID = "";
String _ipAddress = "";


void ApiManager::connectToMqtt(Config &config) {
  Serial.println("=========================== SERVIDOR MQTT ===========================");
  Serial.print("Conectando ao servidor MQTT");

  mqttClient.setServer(config.mqtt_url, config.mqtt_port);
  mqttClient.setKeepAlive(300000);
  mqttClient.setCallback(onMessage);
  
  int attempts = 0;
  String clientId = "ESP32Client-" + String(config.name);  // Gerar um clientId único
  
  // Loop de tentativa de conexão
  while (!mqttClient.connected() && attempts < 5) {  // Removido o argumento da função connected()
    Serial.print(".");
    
    // Tentativa de conexão usando o clientId
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("\nConectado ao servidor MQTT");
      status.mqtt = true;
    } else {
      Serial.print("Falha ao conectar ao servidor MQTT, rc=");
      Serial.print(mqttClient.state());
      status.mqtt = false;
      delay(500);  // Aguarde um pouco antes de tentar novamente
    }

    attempts++;
  }

  Serial.println("=========================== SERVIDOR MQTT ===========================");
}


void ApiManager::handleMqttServer(Config &config){
  mqttClient.loop();

  String topic = "/environments/" + String(config.name);
  mqttClient.subscribe(topic.c_str());
}

void ApiManager::connectToServer(Config &config) {
  HTTPClient http;
  Serial.println("=========================== SERVIDOR ===========================");
  Serial.println("Conectando ao servidor");
  
  http.begin(client, String(config.base_url) + "environments/by_key/" + String(config.name));

  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    Serial.println("Conectado ao servidor");
    status.server = true;
    
    String jsonResponse = http.getString();
    DynamicJsonDocument payload(2048);

    DeserializationError error = deserializeJson(payload, jsonResponse);

    config.temperature_min = payload["temperature_min_value"] | config.temperature_min;
    config.temperature_max = payload["temperature_max_value"] | config.temperature_max;
    config.temperature_alert_sound = payload["temperature_alert_sound"] | config.temperature_alert_sound;
    config.temperature_alert_light = payload["temperature_alert_light"] | config.temperature_alert_light;

    config.humidity_min = payload["humidity_min_value"] | config.humidity_min;
    config.humidity_max = payload["humidity_max_value"] | config.humidity_max;
    config.humidity_alert_sound = payload["humidity_alert_sound"] | config.humidity_alert_sound;
    config.humidity_alert_light = payload["humidity_alert_light"] | config.humidity_alert_light;

    config.mq2_min = payload["mq2_min_value"] | config.mq2_min;
    config.mq2_max = payload["mq2_max_value"] | config.mq2_max;
    config.mq2_alert_sound = payload["mq2_alert_sound"] | config.mq2_alert_sound;
    config.mq2_alert_light = payload["mq2_alert_light"] | config.mq2_alert_light;

    config.noise_min = payload["noise_min_value"] | config.noise_min;
    config.noise_max = payload["noise_max_value"] | config.noise_max;
    config.noise_alert_sound = payload["noise_alert_sound"] | config.noise_alert_sound;
    config.noise_alert_light = payload["noise_alert_light"] | config.noise_alert_light;

    config.ldr_min = payload["ldr_min_value"] | config.ldr_min;
    config.ldr_max = payload["ldr_max_value"] | config.ldr_max;
    config.ldr_alert_sound = payload["ldr_alert_sound"] | config.ldr_alert_sound;
    config.ldr_alert_light = payload["ldr_alert_light"] | config.ldr_alert_light;

    config.presence_alert_sound = payload["presence_alert_sound"] | config.presence_alert_sound;
    config.presence_alert_light = payload["presence_alert_light"] | config.presence_alert_light;

    config.alert_sound = payload["sound_alert"] | config.alert_sound;
    config.alert_light = payload["light_alert"] | config.alert_light;

  } else {
    Serial.println("Falha ao conectar ao servidor");
    status.server = false;
  }

  http.end();
  Serial.println("=========================== SERVIDOR ===========================");

  delay(1000);
}

void ApiManager::sendDataToMqtt(Config &config, String payload){
  if (!mqttClient.connected()) {
    connectToMqtt(config);
  }

  bool status = mqttClient.publish(config.mqtt_topic, payload.c_str());
  Serial.println("Publicando no tópico: " + String(config.mqtt_topic) + " - " + payload);
  Serial.println("Status: " + status ? "Publicado" : "Falha ao publicar");
}

String ApiManager::getRequest(const char* serverName) {
    HTTPClient http;
    String payload;

    http.begin(client, serverName);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        payload = http.getString();
    } else {
        payload = "Error on HTTP request";
    }

    http.end();
    return payload;
}

String ApiManager::postRequest(Config &config, const String& jsonPayload, String sensor_name) {
  HTTPClient http;
  String payload;

  http.begin(client, String(config.base_url) + "environments/" + String(config.name) + "/sensors/" + sensor_name);

  http.addHeader("Content-Type", "application/json");
  http.POST(jsonPayload);

  http.end();
  return "";
}

void ApiManager::sendTemperatureValue(Config &config) {
  float temperatureValue = readTemperature();

  if (_temperature == temperatureValue)
    return;

  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"temperature\",";
  payload += "\"status\":\"" + String(status.temperature) + "\",";
  payload += "\"value\":" + String(temperatureValue);
  payload += "}";

  sendDataToMqtt(config, payload);
    
  _temperature = temperatureValue;
}

void ApiManager::sendHumidityValue(Config &config)
{
  float humidityValue = readHumidity();

  if (_humidity == humidityValue)
    return;

  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"status\":\"" + String(status.humidity) + "\",";
  payload += "\"sensor\":\"humidity\",";
  payload += "\"value\":" + String(humidityValue);
  payload += "}";

  sendDataToMqtt(config, payload);
  
  _humidity = humidityValue;
}

void ApiManager::sendMQ2Value(Config &config) {
  float mq2Value = readMQ2();

  if (roundf(mq2Value * 2.0f) / 2.0 == _mq2) 
    return;
  
  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"mq2\",";
  payload += "\"status\":\"" + String(status.mq2) + "\",";
  payload += "\"value\":" + String(mq2Value);
  payload += "}";

  sendDataToMqtt(config, payload);

  _mq2 = roundf(mq2Value * 2.0f) / 2.0;
}

void ApiManager::sendNoiseValue(Config &config){
  int noiseValue = readNoise();
  if (_noise == noiseValue)
    return;

  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"noise\",";
  payload += "\"status\":\"" + String(status.noise) + "\",";
  payload += "\"value\":" + String(noiseValue);
  payload += "}";

  sendDataToMqtt(config, payload);

  _noise = noiseValue;
}

void ApiManager::sendPresenceValue(Config &config)
{
  bool presenceValue = readPresence();

  if (_presence == presenceValue)
    return;

  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"presence\",";
  payload += "\"status\":\"" + String(status.presence) + "\",";
  payload += "\"value\":" + String(presenceValue);
  payload += "}";

  sendDataToMqtt(config, payload);
  
  _presence = presenceValue;
}

void ApiManager::sendLDRValue(Config &config)
{
  int ldrValue = readLDR();

  if (_ldr == ldrValue)
    return;

  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"ldr\",";
  payload += "\"status\":\"" + String(status.ldr) + "\",";
  payload += "\"value\":" + String(ldrValue);
  payload += "}";

  sendDataToMqtt(config, payload);


  _ldr = ldrValue;
}

void ApiManager::sendWifiValue(Config &config){
  String wifiSSID = WiFi.SSID();
  if (_wifiSSID == wifiSSID)
    return;

  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"wifi\",";  
  payload += "\"status\":\"" + String(status.wifi) + "\",";
  payload += "\"value\":\"" + String(config.ssid) + "\"";
  payload += "}";

  sendDataToMqtt(config, payload);


  _wifiSSID = wifiSSID;
}

void ApiManager::sendIpValue(Config &config){
  String ip = getWifiIP();
  if (_ipAddress == ip)
    return;

  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"ip\",";
  payload += "\"status\":\"" + String(status.wifi) + "\",";
  payload += "\"value\":\"" + String(ip) + "\"";
  payload += "}";

  sendDataToMqtt(config, payload);
  
  _ipAddress = ip;
}

void ApiManager::sendMemoryValue(Config &config)
{
  int memoryValue = readMemoryUsage();
  
  if (_memory == memoryValue)
    return;
  
  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"memory\",";
  payload += "\"status\":\"" + String(true) + "\",";
  payload += "\"value\":\"" + String(memoryValue) + "\"";
  payload += "}";

  sendDataToMqtt(config, payload);

  _memory = memoryValue;
}

void ApiManager::sendCpuValue(Config &config){
  int cpuValue = readCpuUsage();
  
  if (_cpu == cpuValue)
    return;
  
  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"cpu\",";
  payload += "\"status\":\"" + String(true) + "\",";
  payload += "\"value\":\"" + String(cpuValue) + "\"";
  payload += "}";

  sendDataToMqtt(config, payload);
  
  _cpu = cpuValue;
}

void ApiManager::sendUptimeValue(Config &config){
  unsigned long uptimeValue = millis(); 

  if (_uptime == uptimeValue)
    return;
  
  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"uptime\",";
  payload += "\"status\":\"" + String(true) + "\",";
  payload += "\"value\":\"" + String(uptimeValue) + "\"";
  payload += "}";

  sendDataToMqtt(config, payload);
  
  _uptime = uptimeValue;
}

void ApiManager::sendInternalTemperatureValue(Config &config){
  float fInternalTemperatureValue = temperatureRead();
  int internalTemperatureValue = (int)fInternalTemperatureValue;

  if (_internal_temperature == internalTemperatureValue)
    return;
  
  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"temp_internal\",";
  payload += "\"status\":\"" + String(true) + "\",";
  payload += "\"value\":\"" + String(internalTemperatureValue) + "\"";
  payload += "}";

  sendDataToMqtt(config, payload);

  _internal_temperature = internalTemperatureValue;
}

void ApiManager::sendVoltageValue(Config &config){
  float voltageValue = readVoltage();

  if (_voltage == voltageValue)
    return;

  String payload = "{";
  payload += "\"environment\":\"" + String(config.name) + "\",";
  payload += "\"sensor\":\"voltage\",";
  payload += "\"status\":\"" + String(true) + "\",";
  payload += "\"value\":\"" + String(voltageValue) + "\"";
  payload += "}";

  sendDataToMqtt(config, payload);

  _voltage = voltageValue;
}

bool ApiManager::pingToServer(Config &config) {
    HTTPClient http;
    http.begin(client, String(config.base_url) + "environments/" + String(config.name) + "/ping");

    int httpResponseCode = http.GET();

    http.end();

    if  (httpResponseCode == -11){
      status.server = false;
    }else {
      status.server = true;
    }

    return httpResponseCode > 0;
}

unsigned long previousTImeInternalValue = 0;
void ApiManager::sendData(Config &config) {
  if(!status.server)
    return;

  sendTemperatureValue(config);
  sendHumidityValue(config);
  sendMQ2Value(config);
  sendNoiseValue(config);
  sendPresenceValue(config);
  sendLDRValue(config);
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousTImeInternalValue >= 5000) {

    sendWifiValue(config);
    sendMemoryValue(config);
    sendCpuValue(config);
    sendUptimeValue(config);
    sendInternalTemperatureValue(config);
    sendVoltageValue(config);
    sendIpValue(config);
    previousTImeInternalValue = currentMillis;
  }
}