#include "SensorsManager.h"

DHT dht(DHTPIN, DHTTYPE);

float readLDR()
{
  float sensorValue = analogRead(D2);
  float ldrValue = sensorValue * (5.0 / 1023.0);
  return ldrValue;
}

float readDHT11()
{
  float temperature = dht.readTemperature(); // Lê a temperatura em Celsius
  if (isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0; // Retorna 0.0 se a leitura falhar
  }
  return temperature;
}

float readMQ2()
{
  int analogValue = analogRead(A0);
  float voltage = analogValue * (5.0 / 1023.0); // Converte o valor analógico para tensão
  return voltage;                               // Ou converte para a unidade desejada, se necessário
}