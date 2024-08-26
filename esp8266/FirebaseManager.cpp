#include "FirebaseManager.h"
#include <FirebaseESP8266.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

FirebaseData fbdo;
FirebaseAuth firebase_auth;
FirebaseConfig firebase_config;

void checkForConfigUpdates(Config &config)
{
  String configName = config.name;
  String path = "/enviroments/" + configName + "/config";
  static unsigned long lastCheck = 0;

  if (millis() - lastCheck >= 1000)
  {
    lastCheck = millis();

    if (Firebase.getJSON(fbdo, path))
    {
      FirebaseJson &json = fbdo.jsonObject();
      FirebaseJsonData jsonData;

      if (json.get(jsonData, "dht_interval") && jsonData.type == "int")
      {
        config.dht_interval = jsonData.intValue;
        saveConfig(config);
      }

      if (json.get(jsonData, "mq2_interval") && jsonData.type == "int")
      {
        config.mq2_interval = jsonData.intValue;
        saveConfig(config);
      }

      if (json.get(jsonData, "presence_interval") && jsonData.type == "boolean")
      {
        config.presence_interval = jsonData.boolValue;
        saveConfig(config);
      }

      if (json.get(jsonData, "ldr_interval") && jsonData.type == "int")
      {
        config.ldr_interval = jsonData.intValue;
        saveConfig(config);
      }

      if (json.get(jsonData, "noise_interval") && jsonData.type == "int")
      {
        config.noise_interval = jsonData.intValue;
        saveConfig(config);
      }

      if (json.get(jsonData, "sound_alert") && jsonData.type == "boolean")
      {
        config.sound_alert = jsonData.boolValue;
        saveConfig(config);
      }
    }
    else
    {
      FirebaseJson json;

      json.set("dht_interval", config.dht_interval);
      json.set("mq2_interval", config.mq2_interval);
      json.set("presence_interval", config.presence_interval);
      json.set("ldr_interval", config.ldr_interval);
      json.set("noise_interval", config.noise_interval);
      json.set("sound_alert", config.sound_alert);

      Firebase.setJSON(fbdo, path, json);
    }
  }
}

void processFloats(String data, float NewValue, String path)
{
  if (Firebase.getString(fbdo, path))
  {
    data = fbdo.stringData();
    int maxSize = 10;
    String tokens[maxSize];
    int tokenCount = 0;

    int startIndex = 0;
    int commaIndex = data.indexOf(',');
    while (commaIndex != -1)
    {
      tokens[tokenCount++] = data.substring(startIndex, commaIndex);
      startIndex = commaIndex + 1;
      commaIndex = data.indexOf(',', startIndex);
    }
    tokens[tokenCount++] = data.substring(startIndex);

    if (tokenCount >= maxSize)
    {
      for (int i = 1; i < maxSize; i++)
      {
        tokens[i - 1] = tokens[i];
      }
      tokenCount = maxSize - 1;
    }
    tokens[tokenCount++] = String(NewValue);

    String result = tokens[0];
    for (int i = 1; i < tokenCount; i++)
    {
      result += "," + tokens[i];
    }

    Firebase.setString(fbdo, path, result);
  }
  else
  {
    Firebase.setString(fbdo, path, "");
  }
}

void processBools(String data, bool NewValue, String path)
{
  if (Firebase.getString(fbdo, path))
  {
    data = fbdo.stringData();
    int maxSize = 10;
    String tokens[maxSize];
    int tokenCount = 0;

    int startIndex = 0;
    int commaIndex = data.indexOf(',');

    while (commaIndex != -1)
    {
      tokens[tokenCount++] = data.substring(startIndex, commaIndex);
      startIndex = commaIndex + 1;
      commaIndex = data.indexOf(',', startIndex);
    }

    tokens[tokenCount++] = data.substring(startIndex);

    if (tokenCount >= maxSize)
    {
      for (int i = 1; i < maxSize; i++)
      {
        tokens[i - 1] = tokens[i];
      }
      tokenCount = maxSize - 1;
    }
    tokens[tokenCount++] = NewValue ? "true" : "false";

    String result = tokens[0];
    for (int i = 1; i < tokenCount; i++)
    {
      result += "," + tokens[i];
    }

    Firebase.setString(fbdo, path, result);
  }
  else
  {
    Firebase.setString(fbdo, path, "");
  }
}

String getCurrentTimestamp(int timezoneOffset)
{
  time_t now = time(nullptr);

  now += timezoneOffset * 3600; // timezoneOffset é em horas, então multiplicamos por 3600 segundos (1 hora)

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

void sendHistory(String path, String name, float data, bool status)
{

  FirebaseJson json;
  json.set("value", data);
  json.set("updated_at", getCurrentTimestamp(-3));
  json.set("status", status);
  Firebase.push(fbdo, path + "/" + name, json);
}

void sendhtValue(String path, Config &config, String path_history)
{
  int timeInterval = config.dht_interval ? config.dht_interval : 1800;
  static unsigned long sendDataPrevMillis = 0;
  if (millis() - sendDataPrevMillis > timeInterval)
  {
    sendDataPrevMillis = millis();
    float temperatureValue = readTemperature();
    float humidityValue = readHumidity();

    FirebaseJson json;
    json.set("name", "Temperatura");
    json.set("status", status.temperature);
    json.set("value", temperatureValue);
    json.set("updated_at", getCurrentTimestamp(-3));
    Firebase.set(fbdo, path + "sensors/temperature", json);

    json.set("name", "Umidade");
    json.set("status", status.humidity);
    json.set("value", humidityValue);
    json.set("updated_at", getCurrentTimestamp(-3));
    Firebase.set(fbdo, path + "sensors/humidity", json);

    processFloats("", temperatureValue, path + "temperature_history");
    processFloats("", humidityValue, path + "humidity_history");

    sendHistory("/history/" + path_history, "temperature", temperatureValue, status.temperature);
    sendHistory("/history/" + path_history, "humidity", temperatureValue, status.humidity);
  }
}

void sendMQ2Value(String path, Config &config, String path_history)
{
  int timeInterval = config.mq2_interval ? config.mq2_interval : 1800;
  static unsigned long sendDataPrevMillis = 0;
  if (millis() - sendDataPrevMillis > 1500)
  {
    sendDataPrevMillis = millis();
    float mq2Value = readMQ2();

    FirebaseJson json;
    json.set("name", "Gases");
    json.set("status", status.mq2);
    json.set("value", mq2Value);
    json.set("updated_at", getCurrentTimestamp(-3));
    Firebase.set(fbdo, path + "mq2", json);

    processFloats("", mq2Value, path + "mq2_history");
    sendHistory("/history/" + path_history, "sensors/mq2", mq2Value, status.mq2);
  }
}

void sendNoiseValue(String path, Config &config, String path_history)
{
  int timeInterval = config.noise_interval ? config.noise_interval : 1800;
  static unsigned long sendDataPrevMillis = 0;
  if (millis() - sendDataPrevMillis > 1600)
  {
    sendDataPrevMillis = millis();
    float noiseValue = random(500, 1500) / 10.0;
    FirebaseJson json;
    json.set("name", "Ruído");
    json.set("status", true);
    json.set("value", noiseValue);
    json.set("updated_at", getCurrentTimestamp(-3));
    Firebase.set(fbdo, path + "sensors/noise", json);
    processFloats("", noiseValue, path + "noise_history");

    sendHistory("/history/" + path_history, "noise", noiseValue, true);
  }
}

void sendPresenceValue(String path, Config &config, String path_history)
{
  int timeInterval = config.presence_interval ? config.presence_interval : 1800;
  static unsigned long sendDataPrevMillis = 0;
  if (millis() - sendDataPrevMillis > 1700)
  {
    sendDataPrevMillis = millis();
    bool presenceValue = readPresence();
    FirebaseJson json;
    json.set("name", "Presença");
    json.set("status", true);
    json.set("value", presenceValue);
    json.set("updated_at", getCurrentTimestamp(-3));
    Firebase.set(fbdo, path + "sensors/presence", json);

    processBools("", presenceValue, path + "presence_history");

    sendHistory("/history/" + path_history, "presence", presenceValue, true);
  }
}

void sendLDRValue(String path, Config &config, String path_history)
{
  int timeInterval = config.ldr_interval ? config.ldr_interval : 1800;
  static unsigned long sendDataPrevMillis = 0;

  if (millis() - sendDataPrevMillis > timeInterval)
  {
    sendDataPrevMillis = millis();
    float ldrValue = readLDR();
    FirebaseJson json;
    json.set("name", "Luminosidade");
    json.set("status", status.ldr);
    json.set("value", ldrValue);
    Firebase.set(fbdo, path + "sensors/ldr", json);

    processFloats("", ldrValue, path + "ldr_history");
    sendHistory("/history/" + path_history, "ldr", ldrValue, status.ldr);
  }
}

void setWifiValue(String path, Config &config)
{
  String wifiSSID = WiFi.SSID();
  String ipAddress = WiFi.localIP().toString();
  FirebaseJson json;
  json.set("name", "Conexão WiFi");
  json.set("value", wifiSSID);
  json.set("status", WiFi.status() == WL_CONNECTED);
  json.set("updated_at", getCurrentTimestamp(-3));
  json.set("extra", ipAddress);
  Firebase.set(fbdo, path + "sensors/wifi", json);
}

void sendData(Config &config)
{
  String configName = config.name;
  String basePath = "/enviroments/" + configName + "/";

  sendhtValue(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-3));
  sendMQ2Value(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-3));
  sendNoiseValue(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-3));
  sendPresenceValue(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-3));
  sendLDRValue(basePath, config, configName);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-3));
  setWifiValue(basePath, config);
  Firebase.setString(fbdo, basePath + "updated_at", getCurrentTimestamp(-3));
}
