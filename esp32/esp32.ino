#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "ROOMBOX-1234";
const char *password = "12345678";

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

void createAccessPoint()
{
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);
}

void setup() {
    Serial.begin(115200);

    createAccessPoint();

    server.on("/", handle_OnConnect);
  
    server.begin();
}

void loop() {
    server.handleClient();
}

void handle_OnConnect() {
    server.send(200, "text/html", SendHTML()); 
}

String SendHTML() {
    return("<!doctype html><html><head></head><body><p>Hello</p></body></html>");
}