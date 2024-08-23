#include "FirebaseManager.h"
#include <FirebaseESP8266.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

FirebaseData fbdo;
FirebaseAuth firebase_auth;
FirebaseConfig firebase_config;

String processFloats(String data, float NewValue)
{
  // Dividir a string em valores individuais
  int maxSize = 10;
  String tokens[maxSize];
  int tokenCount = 0;

  int startIndex = 0;
  int commaIndex = data.indexOf(',');

  // Extraí todos os valores da string
  while (commaIndex != -1)
  {
    tokens[tokenCount++] = data.substring(startIndex, commaIndex);
    startIndex = commaIndex + 1;
    commaIndex = data.indexOf(',', startIndex);
  }
  // Adiciona o último valor, que não tem uma vírgula após ele
  tokens[tokenCount++] = data.substring(startIndex);

  // Adiciona o novo valor à lista
  if (tokenCount >= maxSize)
  {
    // Remove o valor mais antigo se houver mais de maxSize valores
    for (int i = 1; i < maxSize; i++)
    {
      tokens[i - 1] = tokens[i];
    }
    tokenCount = maxSize - 1;
  }
  tokens[tokenCount++] = String(NewValue);

  // Reconstrua a string com os valores atualizados
  String result = tokens[0];
  for (int i = 1; i < tokenCount; i++)
  {
    result += "," + tokens[i];
  }

  return result;
}

String processDates(String data, String newDate)
{
  // Dividir a string em valores individuais
  int maxSize = 10;
  String tokens[maxSize];
  int tokenCount = 0;

  int startIndex = 0;
  int commaIndex = data.indexOf(',');

  // Extraí todos os valores da string
  while (commaIndex != -1)
  {
    tokens[tokenCount++] = data.substring(startIndex, commaIndex);
    startIndex = commaIndex + 1;
    commaIndex = data.indexOf(',', startIndex);
  }
  // Adiciona o último valor, que não tem uma vírgula após ele
  tokens[tokenCount++] = data.substring(startIndex);

  // Adiciona a nova data à lista
  if (tokenCount >= maxSize)
  {
    // Remove a data mais antiga se houver mais de maxSize valores
    for (int i = 1; i < maxSize; i++)
    {
      tokens[i - 1] = tokens[i];
    }
    tokenCount = maxSize - 1;
  }
  tokens[tokenCount++] = newDate;

  // Reconstrua a string com as datas atualizadas
  String result = tokens[0];
  for (int i = 1; i < tokenCount; i++)
  {
    result += "," + tokens[i];
  }

  return result;
}

String processBools(String data, bool NewValue)
{
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

  return result;
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

void sendData(Config &config)
{
  String configName = config.name;
  String basePath = "/enviroments/" + configName + "/current/";
  String historyPath = "/enviroments/" + configName + "/history/";

  // Ler os valores dos sensores
  float temperatureValue = readTemperature();  // Leitura da temperatura do DHT11
  float humidityValue = readHumidity();        // Leitura da temperatura do DHT11
  float mq2Value = readMQ2();                  // Leitura do MQ2
  float presenceValue = readPresence();        // Leitura do MQ2
  float noiseValue = random(500, 1500) / 10.0; // Exemplo: valores entre 50.0 e 150.0
  float ldrValue = readLDR();
  String ipAddress = WiFi.localIP().toString();
  String wifiSSID = WiFi.SSID();
  String lastUpdate = getCurrentTimestamp(-3);

  Firebase.setFloat(fbdo, basePath + "temperature", temperatureValue);
  Firebase.setFloat(fbdo, basePath + "humidity", humidityValue);
  Firebase.setFloat(fbdo, basePath + "mq2", mq2Value);
  Firebase.setBool(fbdo, basePath + "presence", presenceValue);
  Firebase.setFloat(fbdo, basePath + "ldr", ldrValue);
  Firebase.setFloat(fbdo, basePath + "noise", noiseValue);
  Firebase.setString(fbdo, basePath + "ip", ipAddress);
  Firebase.setString(fbdo, basePath + "wifi", wifiSSID);
  Firebase.setString(fbdo, basePath + "last_update", lastUpdate);

  String data;
  if (Firebase.getString(fbdo, basePath + "temperature_history"))
  {
    data = fbdo.stringData();
    Serial.println(data);
    String processedData = processFloats(data, temperatureValue);
    Serial.println(processedData);
    Firebase.setString(fbdo, basePath + "temperature_history", processedData);
  }

  if (Firebase.getString(fbdo, basePath + "humidity_history"))
  {
    data = fbdo.stringData();
    Serial.println(data);
    String processedData = processFloats(data, humidityValue);
    Serial.println(processedData);
    Firebase.setString(fbdo, basePath + "humidity_history", processedData);
  }

  if (Firebase.getString(fbdo, basePath + "mq2_history"))
  {
    data = fbdo.stringData();
    Serial.println(data);
    String processedData = processFloats(data, mq2Value);
    Serial.println(processedData);
    Firebase.setString(fbdo, basePath + "mq2_history", processedData);
  }

  if (Firebase.getString(fbdo, basePath + "presence_history"))
  {
    data = fbdo.stringData();
    Serial.println(data);
    String processedData = processFloats(data, presenceValue);
    Serial.println(processedData);
    Firebase.setString(fbdo, basePath + "presence_history", processedData);
  }

  if (Firebase.getString(fbdo, basePath + "ldr_history"))
  {
    data = fbdo.stringData();
    Serial.println(data);
    String processedData = processFloats(data, ldrValue);
    Serial.println(processedData);
    Firebase.setString(fbdo, basePath + "ldr_history", processedData);
  }

  if (Firebase.getString(fbdo, basePath + "updated_at_history"))
  {
    data = fbdo.stringData();
    String processedData = processDates(data, lastUpdate);
    Serial.println(processedData);
    Firebase.setString(fbdo, basePath + "updated_at_history", processedData);
  }
  // Construir o JSON com os valores atuais e timestamp
  // FirebaseJson json;
  //  json.set("temperature", temperatureValue);
  // json.set("humidity", humidityValue);
  //  json.set("mq2", mq2Value);
  //  json.set("noise", noiseValue);
  //  json.set("presence", presenceValue);
  //  json.set("ldr", ldrValue);
  //  json.set("ip", ipAddress);
  //  json.set("wifi", wifiSSID);
  //  json.set("last_update", lastUpdate);

  // Adicionar o novo registro ao histórico
  // Serial.printf("Append to history... %s\n", Firebase.push(fbdo, historyPath, json) ? "ok" : fbdo.errorReason().c_str());

  Serial.println();
}
