#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include "ConfigManager.h"
#include "FirebaseManager.h"
// #include "HtmlPage.h"

#define LED_PIN LED_BUILTIN
#define BUTTON_PIN D3

Config config;
const char *ssid = "ESP8266 Access Point"; // Nome da rede WiFi que será criada
const char *password = "123456789";        // Senha para se conectar nesta rede

ESP8266WebServer server(80);

const char MAIN_page[] PROGMEM = R"=====(
  <h1>Hello World</h1>
)=====";

String listSSID()
{
  String index = (const __FlashStringHelper *)MAIN_page; // Leia o conteúdo HTML
  String networks = "";
  int n = WiFi.scanNetworks();

  if (n > 0)
  {
    networks = "<tbody>";
    for (int i = 0; i < n; ++i)
    {
      String ssid = WiFi.SSID(i);
      networks += "<tr>";
      networks += "<td>";
      networks += ssid;
      networks += "</td>";
      networks += "<td>";
      networks += WiFi.RSSI(i);
      networks += "</td>";
      networks += "<td>";
      networks += WiFi.channel(i);
      networks += "</td>";
      networks += "<td>";
      networks += (WiFi.encryptionType(i) == ENC_TYPE_NONE ? "Aberta" : "Fechada");
      networks += "</td>";
      networks += "<td style='text-align: right;'><button  type='button' class='button button-";
      if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid)
      {
        networks += "connected' onclick=\"toggleDisconnect(this, '";
        networks += ssid;
        networks += "')\">Desconectar";
      }
      else
      {
        networks += "-to-connected' onclick=\"toggleConnection(this, '";
        networks += ssid;
        networks += "')\">Conectar";
      }
      networks += "</button></td></tr>";
    }
    networks += "</tbody>";
  }

  Serial.println(config.firebase_api_key);
  Serial.println(config.name);

  index.replace("<tbody><tr><td colspan=\"5\"> Nenhuma rede disponível</td></tr></tbody>", networks);

  index.replace("<input class='text-field' type='text' name='name' maxlength='32' placeholder='Nome do dispositivo'>", "<input class='text-field' type='text' name='name' length=32 placeholder='Name' value='" + String(config.name) + "'>");
  index.replace("<input class='text-field' type='text' name='firebase_api_key' maxlength='64' placeholder='Firebase API KEY'>", "<input class='text-field' type='text' name='firebase_api_key' maxlength='64' placeholder='Firebase API KEY' value='" + String(config.firebase_api_key) + "'>");
  index.replace("<input class='text-field' type='text' name='firebase_data_base_url' maxlength='128' placeholder='Firebase Base URL'>", "<input class='text-field' type='text' name='firebase_data_base_url' maxlength='128' placeholder='Firebase Base URL' value='" + String(config.firebase_data_base_url) + "'>");
  index.replace("<input class='text-field' type='text' name='firebase_email' maxlength='64' placeholder='Firebase Email'>", "<input class='text-field' type='text' name='firebase_email' maxlength='64' placeholder='Firebase email' value='" + String(config.firebase_email) + "'>");
  index.replace("<input class='text-field' type='password' name='firebase_password' maxlength='64' placeholder='Firebase Senha'>", "<input class='text-field' type='password' name='firebase_password' maxlength='64' placeholder='firebase Password' value='" + String(config.firebase_password) + "'>");
  return index;
}

void handleRoot()
{
  String index = listSSID(); // Leia o conteúdo HTML

  server.send(200, "text/html", index); // Enviar pagina Web
}

void connectToWiFi(String ssidWifi, String passwordWifi)
{
  if (ssidWifi.equals("") || passwordWifi.equals(""))
  {
    return;
  }

  int count = 0;
  WiFi.begin(ssidWifi.c_str(), passwordWifi.c_str()); // Conecta com seu roteador

  // Espera por uma conexão
  while (count < 15)
  {
    delay(500);
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED)
    {
      strncpy(config.ssid, ssidWifi.c_str(), sizeof(config.ssid) - 1);
      strncpy(config.password, passwordWifi.c_str(), sizeof(config.password) - 1);
      saveConfig(config);

      Serial.println("");
      Serial.println("");

      // Se a conexão ocorrer com sucesso, mostre o endereço IP no monitor serial
      String responsePage = (const __FlashStringHelper *)MAIN_page; // Leia o conteúdo HTML
      responsePage.replace("<br><br>", "<p id='status'>Conectado!</p>");
      server.send(200, "text/html", responsePage);
      return;
    }
    else if (WiFi.status() == WL_CONNECT_FAILED)
    {
      String responsePage = (const __FlashStringHelper *)MAIN_page;
      responsePage.replace("<br><br>", "<p id='status' style='color:red;'>Falha na conexão.</p>");
      server.send(200, "text/html", responsePage);
    }
    count++;
  }
  String responsePage = (const __FlashStringHelper *)MAIN_page;
  responsePage.replace("<br><br>", "<p id='status' style='color:red;'>Erro.</p>");
  server.send(200, "text/html", responsePage);
  return;
}

void handleFormConnection()
{
  delay(1000);
  String ssidWifi = server.arg("ssid");
  String passwordWifi = server.arg("password");

  if (ssidWifi.equals("") && passwordWifi.equals(""))
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      handleFormDisconect();
    }
  }

  server.on("/", HTTP_POST, handleRoot);

  if (!ssidWifi.equals("") && !passwordWifi.equals(""))
  {
    connectToWiFi(ssidWifi, passwordWifi);
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    saveConfig(config);
  }
}

void handleFormDisconect()
{
  strcpy(config.password, "");
  strcpy(config.ssid, "");
  saveConfig(config);
  Serial.println("Desconectado!");
  printConfig(config);
  WiFi.disconnect();

  enterConfigMode(config);
  IPAddress ip = WiFi.softAPIP();
  server.sendHeader("Location", "http://" + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]) + "/", true);
  server.send(302, "text/plain", "Redirecionando...");
}

void handleFormCredentials()
{
  String name = server.arg("name");
  String firebase_api_key = server.arg("firebase_api_key");
  String firebase_data_base_url = server.arg("firebase_data_base_url");
  String firebase_email = server.arg("firebase_email");
  String firebase_password = server.arg("firebase_password");

  strncpy(config.name, name.c_str(), sizeof(config.name) - 1);
  strncpy(config.firebase_api_key, firebase_api_key.c_str(), sizeof(config.firebase_api_key) - 1);
  strncpy(config.firebase_data_base_url, firebase_data_base_url.c_str(), sizeof(config.firebase_data_base_url) - 1);
  strncpy(config.firebase_email, firebase_email.c_str(), sizeof(config.firebase_email) - 1);
  strncpy(config.firebase_password, firebase_password.c_str(), sizeof(config.firebase_password) - 1);

  saveConfig(config);
  handleRoot();
}

void setupAp()
{
  WiFi.softAP(ssid, password);

  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/action_new_connection", handleFormConnection);
  server.on("/action_form_credentials", handleFormCredentials);

  server.begin();
  Serial.println("Servidor HTTP iniciado");
  Serial.print("WiFi: ");
  Serial.println(ssid);

  Serial.println("Senha: 123456789");
  Serial.print("Acesse ");

  Serial.print(WiFi.softAPIP());

  Serial.println(" para configurar o ESP8266");
  server.handleClient(); // Trata requisições de clientes
}

void enterConfigMode(Config &config)
{
  Serial.println("Entrando no modo de configuração...");
  config.configMode = true;
  delay(1000); // Aguarda 1 segundo para evitar múltiplos cliques
  setupAp();
}

void exitConfigMode(Config &config)
{
  Serial.println("Saido do mode de configuração...");
  config.configMode = false;
  saveConfig(config);
}

void handleButtonConfig(Config &config)
{
  static unsigned long buttonPressTime = 0;
  static bool buttonWasPressed = false;

  if (digitalRead(BUTTON_PIN) == LOW) // Botão pressionado
  {
    if (!buttonWasPressed)
    {
      buttonPressTime = millis(); // Inicia a contagem do tempo
      buttonWasPressed = true;
    }
    else if (millis() - buttonPressTime >= 5000)
    { // Pressionado por 5 segundos
      if (config.configMode == false)
      {
        enterConfigMode(config); // Entra no modo de configuração
      }
      else
      {
        exitConfigMode(config); // Sai do modo de configuração
      }
      buttonPressTime = millis(); // Reinicia o tempo para evitar reexecução imediata
    }
  }
  else // Botão liberado
  {
    if (buttonWasPressed)
    {
      buttonPressTime = 0; // Reseta o contador
      buttonWasPressed = false;
    }
  }

  // Piscar o LED se estiver no modo de configuração
  if (config.configMode)
  {
    digitalWrite(LED_PIN, millis() % 500 < 250 ? LOW : HIGH); // Pisca a cada 500ms
  }
  else
  {
    digitalWrite(LED_PIN, HIGH); // LED desligado
  }
}

int sendDataPrevMillis = 0;
int count = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  EEPROM.begin(512);
  loadConfig(config);
  connectToWiFi(config.ssid, config.password);
  connectToFirebase(config);
  // enterConfigMode(config);
  printConfig(config);
  initSensors();
}

void loop()
{

  handleButtonConfig(config);

  // if (config.configMode) {
  server.handleClient();
  // }

  if (WiFi.status() != WL_CONNECTED)
  {
    enterConfigMode(config);
  }

  if ((millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    if (config.configMode == false)
    {
      sendData(config);
    }
    // delay(5000);
  }
}