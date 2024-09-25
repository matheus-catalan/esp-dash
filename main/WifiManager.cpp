#include "WifiManager.h"
#include "ConfigManager.h"


extern Config config;

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void setupAp() { 
  Serial.println("=========================== PONTO DE ACESSO ===========================");
  WiFi.softAP("ESP Access Point", "12345");
  WiFi.softAPConfig(local_ip, gateway, subnet);

  Serial.println("Servidor HTTP iniciado");
  Serial.print("WiFi: " + String("ESP Access Point"));
  Serial.println("Senha: " + String("12345"));
  Serial.println("Acesse " + String("http://") + WiFi.softAPIP() + " para configurar o dispositivo.");

  delay(1000);
  Serial.println("=========================== PONTO DE ACESSO ===========================");
}

bool connectToWiFi(String ssidWifi, String passwordWifi) {
  Serial.println("=========================== Wi-Fi ===========================");
  Serial.print("Conectando ao Wi-Fi ");
  int maxAttempts = 15;
  int attempts = 0;
  WiFi.begin(ssidWifi.c_str(), passwordWifi.c_str());

  while (attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nConectado ao Wi-Fi");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    else if (WiFi.status() == WL_CONNECT_FAILED)
    {
      Serial.println("\nfalha ao conectar ao wifi");
    }
    attempts++;
  }

  Serial.println("=========================== Wi-Fi ===========================");
}

void setupNetwork()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    setupAp();
    return;
  }
}

String getWifiStatus()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return "Conectado";
  }
  else if (WiFi.status() == WL_CONNECT_FAILED)
  {
    return "Falha ao conectar";
  }
  else
  {
    return "AP";
  }
}

String getWifiSSID()
{
  return WiFi.SSID();
}

String getWifiIP()
{
  return WiFi.localIP().toString();
}

String getPasswd()
{
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == (WIFI_STA + WIFI_AP))
  {
    return "12345";
  }
  else
  {
    return "";
  }
}