#include "SensorsManager.h"
#include <MQUnifiedsensor.h>
DHT dht(DHTPIN, DHTTYPE);
MQUnifiedsensor MQ2(MQ2BOARD, MQ2VOLTAGERESOLUTION, MQ2ADCBITRESOLUTION, MQ2PIN, MQ2TYPE);
sensorsStatus status;

void initSensors()
{
  dht.begin();
  MQ2.setRegressionMethod(1);
  MQ2.setA(4.8387);
  MQ2.setB(-2.68);
  MQ2.init();
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++)
  {
    MQ2.update();
    calcR0 += MQ2.calibrate(MQ2RATIOCLEANAIR);
    Serial.print(".");
  }
  MQ2.setR0(calcR0 / 10);
  if (isinf(calcR0) || isnan(calcR0))
  {
    status.mq2 = false;
  }
  MQ2.serialDebug(true);

  pinMode(PRESENCEPIN, INPUT);
}
float readLDR()
{
  float sensorValue = analogRead(D2);
  float ldrValue = sensorValue * (5.0 / 1023.0);
  return ldrValue;
}

float readTemperature()
{
  float temperature = dht.readTemperature(); // Lê a temperatura em Celsius
  if (isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0; // Retorna 0.0 se a leitura falhar
  }
  return temperature;
}

float readHumidity()
{
  float humidity = dht.readHumidity(); // Lê a umidade relativa
  if (isnan(humidity))
  {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0; // Retorna 0.0 se a leitura falhar
  }
  return humidity;
}

float readMQ2()
{
  MQ2.update();
  MQ2.readSensor();
  float ppm = MQ2.readSensor();
  if (isinf(ppm) || isnan(ppm))
  {
    status.mq2 = false;
    ppm = 0;
  }
  return ppm;
}

bool readPresence(){
  if(digitalRead(PRESENCEPIN) == HIGH){
    return true;
  }
  //LOW : nenhum movimento detectado
  else{
    return false;
  } 

}