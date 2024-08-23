#include "HtmlPage.h"
#include <Arduino.h>

const char MAIN_page[] PROGMEM = R"=====(
  <h1>Hello World</h1>
)=====";

const char *getMainPage()
{
  return MAIN_page;
}
