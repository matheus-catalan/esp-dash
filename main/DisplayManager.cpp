#if defined(ESP8266)
#else
#include <Wire.h>
#include "SSD1306Wire.h"
#include "pins_arduino.h"
#include "DisplayManager.h"

SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED);

#endif

String getCurrentTimestamp(long timezoneOffset) {
  configTime(timezoneOffset, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Falha ao obter o tempo";
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

  return String(buffer);
}

void VextON(void) {
  #if defined(ESP8266)
    return;
  #endif
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) {
  #if defined(ESP8266)
    return;
  #endif
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}

void displayReset(void) {
  #if defined(ESP8266)
    return;
  #endif

  pinMode(RST_OLED, OUTPUT);
  digitalWrite(RST_OLED, HIGH);
  delay(1);
  digitalWrite(RST_OLED, LOW);
  delay(1);
  digitalWrite(RST_OLED, HIGH);
  delay(1);
}

void setupDisplay() {
  #if defined(ESP8266)
    return;
  #endif
  VextON();
  displayReset();

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.display();
}

void drawProgressBar(int progress, String text) {
  #if defined(ESP8266)
    return;
  #endif
  for(int i = progress - 10; i <= progress; i++) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawProgressBar(0, 32, 120, 10, i);

    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, String(i) + "%");
    
    display.drawString(64, 45, text);
    
    display.display();
    delay(100);
  }
}

void drawText(String text) {
  #if defined(ESP8266)
    return;
  #endif


  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 26, text);
  display.display();
}

/*String getCurrentTimestamp(long timezoneOffset, int daylightOffset_sec = 0) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Falha ao obter o tempo";
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

  return String(buffer);
}*/

void drawHomeScreen(String name, String wifi, String ssid, String ip, String db, String passwd) {
    #if defined(ESP8266)
      return;
    #endif

    display.clear();
    display.flipScreenVertically();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    String date = getCurrentTimestamp(-10800);

    int y = 0;

    display.drawString(1, y, "NAME - " + name);
    y += 11;

    display.drawString(1, y, date);
    y += 11; 

    display.drawString(1, y, "WIFI - " + wifi);
    y += 11;  

    display.drawString(1, y, "IP - " + ip);
    y += 11; 



    if (wifi.equals("AP")) {
        display.drawString(1, y, "SSID - " + ssid);
        y += 11; 
        
        display.drawString(1, y, "Passwd - " + passwd);
        y += 11; 
    } else {
      display.drawString(1, y, "DB - " + db);
    }

    display.display();
}
