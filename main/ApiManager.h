#ifndef APIMANAGER_H
#define APIMANAGER_H

#include "ConfigManager.h"
#include "SensorsManager.h"
class ApiManager {
public:
    ApiManager();
    String getRequest(const char* serverName);
    bool pingToServer(Config &config);
    void connectToServer(Config &config);
    void sendData(Config &config);
    String postRequest(Config &config, const String& jsonPayload, String sensor_name);
    String getCurrentTimestamp(long timezoneOffset);
    void sendTemperatureValue(Config &config);
    void sendHumidityValue(Config &config);
    void sendMQ2Value(Config &config);
    void sendNoiseValue(Config &config);
    void sendPresenceValue(Config &config);
    void sendLDRValue(Config &config);
    void sendWifiValue(Config &config);
    void sendMemoryValue(Config &config);
    void sendCpuValue(Config &config);
    void sendUptimeValue(Config &config);
    void sendInternalTemperatureValue(Config &config);
    void sendVoltageValue(Config &config);
    void sendIpValue(Config &config);
    void sendSensorData();
    void connectToMqtt(Config &config);
    void handleMqttServer();
    void sendDataToMqtt(Config &config, String payload);
    void updateConfig(char* topic, byte* payload, unsigned int length);





private:
    WiFiClient client;
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
};

#endif
