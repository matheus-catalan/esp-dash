#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#if defined(ESP8266)
  #include <ESP8266WebServer.h>
#else
  #include <WebServer.h>
#endif

void setupHttpServer();
void handleClient();

#endif
