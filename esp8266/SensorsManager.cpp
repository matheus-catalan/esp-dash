#include "SensorsManager.h"
#include <MQUnifiedsensor.h>
DHT dht(DHTPIN, DHTTYPE);
MQUnifiedsensor MQ2(MQ2BOARD, MQ2VOLTAGERESOLUTION, MQ2ADCBITRESOLUTION, MQ2PIN, MQ2TYPE);
sensorsStatus status;
bool buzzerState = false;
unsigned long lastBuzzerToggleTime = 0;

void initSensors()
{
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();
  MQ2.serialDebug(false);
  MQ2.setRegressionMethod(1);
  MQ2.setA(4.8387);
  MQ2.setB(-2.68);
  MQ2.init();
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++)
  {
    MQ2.update();
    calcR0 += MQ2.calibrate(MQ2RATIOCLEANAIR);
  }
  MQ2.setR0(calcR0 / 10);
  if (isinf(calcR0) || isnan(calcR0))
  {
    status.mq2 = false;
  }
  else
  {
    status.mq2 = true;
  }

  pinMode(PRESENCEPIN, INPUT);
}
float readLDR()
{
  float sensorValue = analogRead(D2);
  float ldrValue = sensorValue * (5.0 / 1023.0);
  status.ldr = true;
  return ldrValue;
}

float readTemperature()
{
  float temperature = dht.readTemperature();
  if (isnan(temperature))
  {
    status.temperature = false;
    return 0.0;
  }
  status.temperature = true;
  return temperature;
}

float readHumidity()
{
  float humidity = dht.readHumidity();
  if (isnan(humidity))
  {
    status.humidity = false;
    return 0.0;
  }
  status.humidity = true;
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
    return 0;
  }
  status.mq2 = true;
  return ppm;
}

bool readPresence()
{
  if (digitalRead(PRESENCEPIN) == HIGH)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void checkSensorStatus(bool sound_alert)
{
  bool hasError = false;

  if (!status.temperature || !status.humidity || !status.mq2)
  {
    Serial.println("Erro ao ler sensores:");
    Serial.print("Temperatura: ");
    Serial.println(status.temperature ? "OK" : "ERRO");
    Serial.print("Umidade: ");
    Serial.println(status.humidity ? "OK" : "ERRO");
    Serial.print("MQ2: ");
    Serial.println(status.mq2 ? "OK" : "ERRO");

    hasError = true;
  }

  if (hasError && sound_alert)
  {
    unsigned long currentMillis = millis();

    if (currentMillis - lastBuzzerToggleTime >= 1000)
    {
      lastBuzzerToggleTime = currentMillis;

      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
    }
  }
  else
  {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = false;
  }
}
