#ifndef WIFIMANAGE_H
#define WIFIMANAGE_H

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

void setupAp();
void setupNetwork();
bool connectToWiFi(String ssidWifi, String passwordWifi);
String getWifiStatus();
String getWifiSSID();
String getWifiIP();
String getPasswd();

#endif