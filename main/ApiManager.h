#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include "ConfigManager.h"
#include "SensorsManager.h"

class ApiManager {
public:
    ApiManager();
    String getRequest(const char* serverName);
    bool pingToServer(Config &config);
    void sendData(Config &config);
    String postRequest(Config &config, const String& jsonPayload, String sensor_name);
    void sendTemperatureValue(Config &config, String path_history);
    String getCurrentTimestamp(long timezoneOffset);

private:
    WiFiClient client;
};

#endif
