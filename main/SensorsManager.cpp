#include "SensorsManager.h"
#include <MQUnifiedsensor.h>
#include "driver/temperature_sensor.h"
#include <ResponsiveAnalogRead.h>


DHT dht(DHTPIN, DHTTYPE);
MQUnifiedsensor MQ2(MQ2BOARD, MQ2VOLTAGERESOLUTION, MQ2ADCBITRESOLUTION, MQ2PIN, MQ2TYPE);
sensorsStatus status;
unsigned long previousMillis = 0;
unsigned long previousMillisLed = 0;
const long interval = 1000;
bool buzzerState = false;
bool ledState = false;
bool alertLight = false;

ResponsiveAnalogRead analogLDR(0, true);
ResponsiveAnalogRead analogPresence(0, true);
ResponsiveAnalogRead analogMq2(0, true);
ResponsiveAnalogRead analogTemperature(0, true);
ResponsiveAnalogRead analogHumidity(0, true);
ResponsiveAnalogRead analogNoise(0, true);
ResponsiveAnalogRead analogVoltage(0, true);

void initSensors() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(DHTPIN, INPUT);
  for(int i = 0; i < 2; i++){
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }

  dht.begin();
  MQ2.setRegressionMethod(1);
  MQ2.setA(658.71);
  MQ2.setB(-2.168);
  MQ2.init();
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ2.update();
    calcR0 += MQ2.calibrate(MQ2RATIOCLEANAIR);
  }

  MQ2.setR0(calcR0 / 10);
  if (isinf(calcR0) || isnan(calcR0)){
    status.mq2 = false;
  } else {
    status.mq2 = true;
  }
  
  pinMode(PRESENCE_PIN, INPUT);
  pinMode(NOISE_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
}

float readLDR() {
  unsigned long other_resistor = 10000;
  float mult_value = 32017200;
  float pow_value = 1.5832;
  bool photocell_on_ground = true;
  
  const int numReadings = 10;
  float total = 0;
  for (int i = 0; i < numReadings; i++) {
    float raw_value = analogRead(LDR_PIN);
    if (raw_value == 4095) raw_value--;
    
    float ratio = (4096.0 / raw_value) - 1;
    unsigned long photocell_resistor = photocell_on_ground ? (other_resistor / ratio) : (other_resistor * ratio);
    
    float lux = mult_value / pow(photocell_resistor, pow_value);
    if (!isnan(lux)) {
      total += lux;
    } else {
      status.ldr = false;
      return 0;
    }
  }
  
  status.ldr = true;

  analogLDR.update(total / numReadings);


  return analogLDR.getValue();
}

float readTemperature(){
  float temperature = dht.readTemperature();
  if (isnan(temperature))
  {
    status.temperature = false;
    return 0.0;
  }
  status.temperature = true;
  analogTemperature.update(temperature);
  return analogTemperature.getValue();
}

float readHumidity() {
  float humidity = dht.readHumidity();
  
  if (isnan(humidity)) {
    status.humidity = false;
    return 0.0;
  }
  status.humidity = true;
  analogHumidity.update(humidity);
  return analogHumidity.getValue();
}

float readMQ2() {
  MQ2.update();
  float ppm = MQ2.readSensor();

  if (isinf(ppm) || isnan(ppm)) {
    status.mq2 = false;
    return 0.0;
  }
  status.mq2 = true;
  analogMq2.update(ppm);
  return analogMq2.getValue();
}

bool readPresence() {
  return digitalRead(PRESENCE_PIN) == HIGH;
}

int readNoise() {
  float sensorValue = analogRead(NOISE_PIN);
  int noiseValue = round(sensorValue * (5.0 / 1023.0));
  if (isnan(noiseValue) || isinf(noiseValue)) {
    status.noise = false;
    return 0;
  } else {
    status.noise = true;
    return noiseValue;
  }
}

int readMemoryUsage() {
  uint32_t totalMemory = ESP.getHeapSize();
  uint32_t freeMemory = ESP.getFreeHeap(); 
  uint32_t usedMemory = totalMemory - freeMemory;

  float usedPercentage = ((float)usedMemory / totalMemory) * 100;

  return (int)usedPercentage;
}

volatile unsigned long idleCounter = 0;
unsigned long previousIdleCounter = 0;
unsigned long previousMillisCPU = 0;

int readCpuUsage() {
  unsigned long currentMillis = millis();
  unsigned long currentIdleCounter = idleCounter;

  unsigned long elapsedTime = currentMillis - previousMillisCPU;
  if (elapsedTime == 0) return 0.0;  // Evita divisão por zero

  unsigned long idleDelta = currentIdleCounter - previousIdleCounter;
  float idlePercentage = ((float)idleDelta / elapsedTime) * 100.0;
  float cpuUsage = 100.0 - idlePercentage;  // A CPU usada é o complemento da CPU ociosa

  previousIdleCounter = currentIdleCounter;
  previousMillisCPU = currentMillis;

  return (int)cpuUsage;
}

int readVoltage() {
  int adcValue = analogRead(VOLTAGE_PIN);  
  float voltage = (adcValue / 4095.0) * 3.3;  // Converte o valor ADC para voltagem (escala de 0 a 3.3V)
  return (int)voltage;
}

void sound_alert() {
  buzzerState = !buzzerState;
  digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
}

void light_alert() {
  unsigned long currentMillisa = millis();

  if (currentMillisa - previousMillisLed >= 1000) {
    previousMillisLed = currentMillisa;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
}

void alert(String sensor_name, bool alert_sound = false, bool alert_light = false) {
  if (!alert_sound && !alert_light) { return; }
  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    Serial.println("===================================== Alerta! =====================================");
    Serial.println(sensor_name);
    Serial.println("===================================== Alerta! =====================================");
    
   
    previousMillis = currentMillis;
    if(alert_sound){
      sound_alert();
    } 

  }

  if (alert_light){
    light_alert();
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void checkTemperature(Config &config) {
  float temperature = readTemperature();
  if (status.temperature) {
    if (temperature < config.temperature_min || temperature > config.temperature_max) {
      alert("Temperatura", config.temperature_alert_sound && config.alert_sound, config.temperature_alert_light && config.alert_light);
    }
  }
}

void checkHumidity(Config &config) {
  if (status.humidity) {
    float humidity = readHumidity();
    if (humidity < config.humidity_min || humidity > config.humidity_max) {
      alert("Umidade", config.humidity_alert_sound, config.humidity_alert_light);
    }
  }
}

void checkLDR(Config &config){
  if (status.ldr) {
    float ldr = readLDR();
    if (ldr < config.ldr_min || ldr > config.ldr_max) {
      alert("LDR", config.ldr_alert_sound, config.ldr_alert_light);
    }
  }
}

void checkMQ2(Config &config) {
  if (status.mq2) {
    float mq2 = readMQ2();
    if (mq2 < config.mq2_min || mq2 > config.mq2_max) {
      alert("MQ2", config.mq2_alert_sound, config.mq2_alert_light);
    }
  }
}

void checkPresence(Config &config) {
  if (readPresence()) {
    alert("Presença", config.presence_alert_sound, config.presence_alert_light);
  }

}

void checkNoise(Config &config) {
  if (status.noise) {
    float noise = readNoise();
    if (noise < config.noise_min || noise > config.noise_max) {
        alert("Ruído", config.noise_alert_sound, config.noise_alert_light);
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

String getServerStatus(){
  if(status.server && status.mqtt){
    return "Conectado";
  } else {
    return "Desconectado";
  }
}