#include "ApiManager.h"

ApiManager::ApiManager() {}

float _temperature = 0.0;

String ApiManager::getCurrentTimestamp(long timezoneOffset) {
  configTime(timezoneOffset, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Falha ao obter o tempo";
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

  return String(buffer);
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

  http.begin(client, String(config.base_url) + "/sensors/" + String(config.name) + "/" + sensor_name);

  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
      payload = http.getString();
  } else {
      payload = "Error on HTTP request";
  }

  http.end();
  return payload;
}

void ApiManager::sendTemperatureValue(Config &config, String path_history)
{
  float temperatureValue = readTemperature();
  if (_temperature == temperatureValue)
    return;

  String jsonPayload = "{\"status\":\"temperature\",\"value\":23.5}";


  String jsonString = "{";
  jsonString += "\"status\":" + String(status.temperature) + ",";
  jsonString += "\"value\":" + String(temperatureValue) + ",";
  jsonString += "\"updated_at\":\"" + getCurrentTimestamp(-10800) + "\"";
  jsonString += "}";

  //[sendHistory("/history/" + path_history, "temperature", temperatureValue, status.temperature);
  _temperature = temperatureValue;
}

bool ApiManager::pingToServer(Config &config) {
    HTTPClient http;
    Serial.println("Pinging to server");

    http.begin(client, String(config.base_url) + String(config.name) + "/ping");
    int httpResponseCode = http.GET();

    http.end();

    return httpResponseCode > 0;
}

void ApiManager::sendData(Config &config) {
  String configName = config.name;

   sendTemperatureValue(config, configName);
   pingToServer(config);
}