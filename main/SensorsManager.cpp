#include "SensorsManager.h"
#include <MQUnifiedsensor.h>
DHT dht(DHTPIN, DHTTYPE);
MQUnifiedsensor MQ2(MQ2BOARD, MQ2VOLTAGERESOLUTION, MQ2ADCBITRESOLUTION, MQ2PIN, MQ2TYPE);
sensorsStatus status;
unsigned long previousMillis = 0;
unsigned long previousMillisLed = 0;
const long interval = 500;
bool buzzerState = false;
bool ledState = false;
bool alertLight = false;

void initSensors()
{
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);

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
  
  pinMode(PRESENCE_PIN, INPUT);

}

float readLDR()
{
  float sensorValue = analogRead(LDR_PIN);
  
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

  if (digitalRead(PRESENCE_PIN) == HIGH) {
    return true;
  } else {
    return false;
  }
}

float readNoise()
{
  float sensorValue = analogRead(NOISE_PIN);

  float noiseValue = sensorValue * (5.0 / 1023.0);
  status.noise = true;
  return noiseValue;
  //return 0.0;
}

uint32_t readMemoryUsage() {
  uint32_t freeMemory = ESP.getFreeHeap();

  return freeMemory;
}

void alert_sound() {
  buzzerState = !buzzerState;
  digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
}

void alert_light() {
  unsigned long currentMillisa = millis();

  if (currentMillisa - previousMillisLed >= 1000) {
    previousMillisLed = currentMillisa;
    ledState = !ledState;
    Serial.println("Led State: " + String(ledState));
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
}

void alert(String sensor_name, bool sound_alert = false, bool light_alert = false)
{
  if (!sound_alert && !light_alert) { return; }
  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    Serial.println("===================================== Alerta! =====================================");
    Serial.println(sensor_name);
    Serial.println("===================================== Alerta! =====================================");
  
    previousMillis = currentMillis;
    if(sound_alert){
      alert_sound();
    } 

  }
  Serial.println("Light Alert: " + String(light_alert));
  if (light_alert){
    alert_light();
  }
}

void checkTemperature(Config &config)
{
  float temperature = readTemperature();
  if (status.temperature) {
    if (temperature < config.temperature_min || temperature > config.temperature_max) {
      alert("Temperatura", config.temperature_alert, config.temperature_light_alert);
    }
  }
}

void checkHumidity(Config &config) {
  if (status.humidity) {
    float humidity = readHumidity();
    if (humidity < config.humidity_min || humidity > config.humidity_max) {
      alert("Umidade", config.humidity_alert, config.humidity_light_alert);
    }
  }
}

void checkLDR(Config &config){
  if (status.ldr) {
    float ldr = readLDR();
    if (ldr < config.ldr_min || ldr > config.ldr_max) {
      alert("LDR", config.ldr_alert, config.ldr_light_alert);
    }
  }
}

void checkMQ2(Config &config) {
  if (status.mq2) {
    float mq2 = readMQ2();
    if (mq2 < config.mq2_min || mq2 > config.mq2_max) {
      alert("MQ2", config.mq2_alert, config.mq2_light_alert);
    }
  }
}

void checkPresence(Config &config) {
  if (readPresence()) {
    alert("Presença", config.presence_alert, config.presence_light_alert);
  }

}

void checkNoise(Config &config) {
  if (status.noise) {
    float noise = readNoise();
    if (noise < config.noise_min || noise > config.noise_max) {
        alert("Ruído", config.noise_alert, config.noise_light_alert);
    }
  }
}

void checkSensorStatus(Config &config)
{
  digitalWrite(BUZZER_PIN, LOW);
  checkTemperature(config);
  checkHumidity(config);
  checkLDR(config);
  checkMQ2(config);
  checkPresence(config);
  checkNoise(config);
}