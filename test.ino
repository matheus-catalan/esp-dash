#include <EEPROM.h>
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "DHT.h"

#define LED_PIN LED_BUILTIN
#define BUTTON_PIN D3

#define EEPROM_START_ADDRESS 0

#define DHTPIN D1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

struct Config
{
  char ssid[32] = "";
  char password[32] = "";
  char name[32] = "";
  char firebase_api_key[64] = "";
  char firebase_data_base_url[128] = "";
  char firebase_email[64] = "";
  char firebase_password[64] = "";
  bool configMode = false;
};

Config config;
const char *ssid = "ESP8266 Access Point"; // Nome da rede WiFi que será criada
const char *password = "123456789";        // Senha para se conectar nesta rede

ESP8266WebServer server(80);

FirebaseData fbdo;

FirebaseAuth firebase_auth;
FirebaseConfig firebase_config;

const char MAIN_page[] PROGMEM = R"=====(
  <!DOCTYPE html>
  <html lang="pt-BR">
  
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 Configuration</title>
    <style>
      body {
        color: #434343;
        font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
        font-size: 14px;
        background-color: #D9D9D9;
        margin-top: 100px;
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
      }
  
      .container {
        display: flex;
        justify-content: center;
        align-items: center;
        flex-direction: column;
        box-shadow: 0 10px 20px rgba(0, 0, 0, 0.19), 0 6px 6px rgba(0, 0, 0, 0.23);
        background-color: #262626;
        border-radius: 10px;
        padding: 30px;
        max-width: 600px;
      }
  
      .section {
        width: 100%;
        padding: 0;
        margin-bottom: 20px;
        /* Adiciona espaçamento entre as sections */
      }
  
      h3 {
        text-align: center;
        margin-bottom: 20px;
        margin-top: 0;
        color: #D9D9D9;
        font-size: 25px;
      }
  
      .field-group {
        margin-bottom: 20px;
      }
  
      .text-field {
        font-size: 15px;
        width: 100%;
        padding: 15px;
        border-radius: 5px;
        border: 1px solid #D9D9D9;
        background-color: transparent;
        color: #D9D9D9;
        box-sizing: border-box;
      }
  
      .text-field:focus {
        border-color: #00bcd4;
        outline: 0;
      }
  
      .button-container {
        text-align: center;
        margin-top: 20px;
        width: 100%;
      }
  
      .button {
        background: #D9D9D9;
        border: none;
        border-radius: 5px;
        color: #000000;
        cursor: pointer;
        font-weight: bold;
        font-size: 16px;
        padding: 15px 0;
        text-align: center;
        text-transform: uppercase;
        width: 100%;
        transition: background 250ms ease;
        padding: 6px 5px;
        font-size: 12px;
      }
  
      #status {
        text-align: center;
        color: #336859;
        font-size: 14px;
      }
  
      td {
        padding: 10px;
        color: #D9D9D9;
        border: 1px solid #B0B0B0;
      }
  
      th {
        text-align: left;
        color: #262626;
        padding: 10px;
        border: 1px solid #B0B0B0;
      }
  
      .button-to-connected {
        color: #000;
        background-color: #D9D9D9;
        border: 1px solid #B0B0B0;
      }
  
      .button-connected {
        color: #000;
        background-color: #FF0000;
  
      }
    </style>
    <script>
      function validateFormNetwork() {
        var password = document.forms["myForm"]["password"].value;
        var status = document.getElementById("statusDiv");
        if (password == "") {
          status.innerHTML = "<p id='status' style='color:red;'>Insira senha.</p>";
          return false;
        }
        else {
          status.innerHTML = "<p id='status'>Conectando...</p>";
          return true;
        }
      }

      function toggleDisconnect(button) {
        var row = button.parentElement.parentElement;

        var form = document.createElement('form');
        form.name = 'networkForm';
        form.method = 'post';
        form.action = '/action_new_connection'; 

        var statusElement = document.createElement('p');
        statusElement.id = 'status';
        statusElement.style.textAlign = 'center';
        statusElement.style.color = '#336859';
        statusElement.style.fontSize = '14px';
        statusElement.style.marginTop = '10px';
  
        // Adiciona os elementos ao DOM
        var td = document.createElement('td');
        td.colSpan = 5;
        td.appendChild(form);
        td.appendChild(statusElement); 

        var newRow = document.createElement('tr');
        newRow.appendChild(td);
        row.parentElement.insertBefore(newRow, row.nextSibling);

        form.submit();
  
        setTimeout(function () {
          statusElement.innerText = 'Desconectado!';

          var buttonToUpdate = row.querySelector('.button-to-connected');
          if (buttonToUpdate) {
            buttonToUpdate.innerText = 'Conectar';
            buttonToUpdate.style.color = '#000';
            buttonToUpdate.disabled = false;
          }
        }, 2000);

        window.location.href = 'http://192.168.4.1/';
      }

      function toggleConnection(button, ssid) {
        var row = button.parentElement.parentElement;
  
        var passwordFieldRow = row.nextElementSibling;
        var passwordFieldExists = passwordFieldRow && passwordFieldRow.querySelector('.password-field');
  
        // Redefine todos os botões "Conectar" para o estado original
        var allButtons = document.querySelectorAll('.button-to-connected');
        allButtons.forEach(function (btn) {
          btn.disabled = false; // Habilita todos os botões
          btn.innerText = 'Conectar';
        });
  
        // Remove todas as linhas de senha abertas, exceto a atual
        var allPasswordRows = document.querySelectorAll('.password-field');
        allPasswordRows.forEach(function (field) {
          var rowToRemove = field.closest('tr');
          if (rowToRemove !== passwordFieldRow) {
            rowToRemove.remove();
  
            var previousRow = rowToRemove.previousElementSibling;
            if (previousRow) {
              var buttonToReset = previousRow.querySelector('.button-to-connected');
              if (buttonToReset) {
                buttonToReset.innerText = 'Conectar';
              }
            }
          }
        });
  
        // Se o campo de senha já existe na linha atual, apenas fecha e retorna
        if (passwordFieldExists) {
          row.parentElement.removeChild(passwordFieldRow);
          button.innerText = 'Conectar';
          updateStatus('');
          return;
        }
  
        // Cria o campo de senha e o botão de confirmação
        var passwordInput = document.createElement('input');
        passwordInput.name = 'password'
        passwordInput.type = 'password';
        passwordInput.placeholder = 'Digite a senha';
        passwordInput.className = 'text-field password-field';
        passwordInput.style.marginTop = '10px';
  
        var confirmButton = document.createElement('button');
        confirmButton.innerText = 'Confirmar';
        confirmButton.className = 'button';
        confirmButton.style.marginTop = '10px';

        var ssIdInput = document.createElement('input');
        ssIdInput.style.display = 'none';
        ssIdInput.value = ssid;
        ssIdInput.name = 'ssid';

        var form = document.createElement('form');
        form.name = 'networkForm';
        form.method = 'post';
        form.action = '/action_new_connection'; 
        form.appendChild(passwordInput);
        form.appendChild(ssIdInput);
        form.appendChild(confirmButton);
  
        // Adiciona o evento de clique ao botão de confirmação
        confirmButton.addEventListener('click', function () {
          // Desativa todos os botões
          allButtons.forEach(function (btn) {
            btn.disabled = true;
          });
          handleConnection(row, passwordInput.value);
        });
  
        // Cria a área de status
        var statusElement = document.createElement('p');
        statusElement.id = 'status';
        statusElement.style.textAlign = 'center';
        statusElement.style.color = '#336859';
        statusElement.style.fontSize = '14px';
        statusElement.style.marginTop = '10px';
  
        // Adiciona os elementos ao DOM
        var td = document.createElement('td');
        td.colSpan = 5;
        td.appendChild(form);
        td.appendChild(statusElement); // Adiciona a área de status abaixo do botão de confirmação
  
        var newRow = document.createElement('tr');
        newRow.appendChild(td);
        row.parentElement.insertBefore(newRow, row.nextSibling);
  
        button.innerText = 'Cancelar';
      }
  
      function connectToNetwork(row, password) {
        var ssid = row.querySelector('td').innerText;
        var statusElement = document.getElementById("status");
        statusElement.innerText = 'Conectando...';
  
        setTimeout(function () {
          statusElement.innerText = 'Conexão estabelecida com sucesso!';
  
          var passwordFieldRow = row.nextElementSibling;
          if (passwordFieldRow) {
            row.parentElement.removeChild(passwordFieldRow);
          }
  
          var buttonToUpdate = row.querySelector('.button-to-connected');
          if (buttonToUpdate) {
            buttonToUpdate.innerText = 'Desconectar';
            buttonToUpdate.style.color = 'red';
            buttonToUpdate.disabled = false;
          }
        }, 2000);
      }
  
      function updateStatus(message) {
        var statusElement = document.getElementById("status");
        if (statusElement) {
          statusElement.innerText = message;
        }
      }
    </script>
  </head>
  
  <body>
    <div class="container">
      <div class="section">
        <h3> Configuração Wi-Fi</h3>
        <div style="display: flex; align-items: center; justify-content: flex-end; margin-bottom: 10px;">
          <button class="button" style="width: 50px;" onclick="">
            <span style="font-size: 20px;">&#x21bb;</span>
          </button>
        </div>
        <div class="field-group">
          <table
            style="width: 100%; border-collapse: separate; border-spacing: 0; border-radius: 10px; overflow: hidden; border: 1px solid #B0B0B0;">
            <thead>
              <tr style="background-color: #D9D9D9;">
                <th>SSID</th>
                <th>Nível</th>
                <th>Canal</th>
                <th>Segurança</th>
                <th style="text-align: left; padding: 10px; border: 1px solid #B0B0B0;"></th>
              </tr>
            </thead>
            <tbody><tr><td colspan="5"> Nenhuma rede disponível</td></tr></tbody>
          </table>
        </div>
      </div>
      <div class="section">
        <h3>Configurações Dispositivo</h3>
        <form action='/action_form_credentials' method='post'> 
          <div class="field-group">
            <input class='text-field' type='text' name='name' maxlength='32' placeholder='Nome do dispositivo'>
          </div>
          <div class="field-group">
            <input class='text-field' type='text' name='firebase_api_key' maxlength='64' placeholder='Firebase API KEY'>
          </div>
          <div class="field-group">
            <input class='text-field' type='text' name='firebase_data_base_url' maxlength='128' placeholder='Firebase Base URL'>
          </div>
          <div class="field-group">
            <input class='text-field' type='text' name='firebase_email' maxlength='64' placeholder='Firebase Email'>
          </div>
          <div class="field-group">
            <input class='text-field' type='password' name='firebase_password' maxlength='64' placeholder='Firebase Senha'>
          </div>
          <div class="field-group" style="display: flex; align-items: center; justify-content: center;">
            <button class="button"
              style="color: #000; background-color: #D9D9D9; font-size: 15px; height: 30px; max-width: 200px; border: 1px solid #B0B0B0;">Salvar</button>
          </div>
        </form>
      </div>
    </div>
  </body>
  </html>
)=====";

void loadConfig(Config &config)
{
  int address = EEPROM_START_ADDRESS;

  EEPROM.get(address, config.ssid);
  address += sizeof(config.ssid);

  EEPROM.get(address, config.password);
  address += sizeof(config.password);

  EEPROM.get(address, config.configMode);
  address += sizeof(config.configMode);

  EEPROM.get(address, config.name);
  address += sizeof(config.name);

  EEPROM.get(address, config.firebase_api_key);
  address += sizeof(config.firebase_api_key);

  EEPROM.get(address, config.firebase_data_base_url);
  address += sizeof(config.firebase_data_base_url);

  EEPROM.get(address, config.firebase_email);
  address += sizeof(config.firebase_email);

  EEPROM.get(address, config.firebase_password);
  address += sizeof(config.firebase_password);
}

void printConfig(const Config &config)
{
  delay(500);
  Serial.println("-----------------------------------------------------");
  // Serial.print("Total heap: ");
  // Serial.println(ESP.getHeapSize());

  // Serial.print("Free heap: ");
  // Serial.println(ESP.getFreeHeap());

  // Serial.print("Max allocatable block: ");
  // Serial.println(ESP.getMaxAllocHeap());
  // Serial.println("Configurações atuais:");
  Serial.print("WiFi SSID: ");
  if (config.ssid[0] != '\0')
  {
    Serial.println(config.ssid);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("WiFi Password: ");
  if (config.password[0] != '\0')
  {
    Serial.println(config.password);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("IP: ");
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Não conectado");
  }
  Serial.print("Config Mode: ");
  Serial.println(config.configMode ? "Yes" : "No");
  Serial.print("Name: ");
  if (config.name[0] != '\0')
  {
    Serial.println(config.name);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("Firebase API Key: ");
  if (config.firebase_api_key[0] != '\0')
  {
    Serial.println(config.firebase_api_key);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("Firebase DB URL: ");
  if (config.firebase_data_base_url[0] != '\0')
  {
    Serial.println(config.firebase_data_base_url);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("Firebase Email: ");
  if (config.firebase_email[0] != '\0')
  {
    Serial.println(config.firebase_email);
  }
  else
  {
    Serial.println("");
  }
  Serial.print("Firebase Password: ");
  if (config.firebase_password[0] != '\0')
  {
    Serial.println(config.firebase_password);
  }
  else
  {
    Serial.println("");
  }
  Serial.println("-----------------------------------------------------");
}

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

void saveConfig(const Config &config)
{
  int address = EEPROM_START_ADDRESS;

  // Salva o SSID
  EEPROM.put(address, config.ssid);
  address += sizeof(config.ssid);

  // Salva a senha
  EEPROM.put(address, config.password);
  address += sizeof(config.password);

  // Salva o modo de configuração
  EEPROM.put(address, config.configMode);
  address += sizeof(config.configMode);

  EEPROM.put(address, config.name);
  address += sizeof(config.name);

  EEPROM.put(address, config.firebase_api_key);
  address += sizeof(config.firebase_api_key);

  EEPROM.put(address, config.firebase_data_base_url);
  address += sizeof(config.firebase_data_base_url);

  EEPROM.put(address, config.firebase_email);
  address += sizeof(config.firebase_email);

  EEPROM.put(address, config.firebase_password);
  address += sizeof(config.firebase_password);

  EEPROM.commit(); // Garante que as mudanças sejam salvas
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

void connectToFirebase(Config &config)
{
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  firebase_config.api_key = config.firebase_api_key;

  firebase_auth.user.email = config.firebase_email;
  firebase_auth.user.password = config.firebase_password;
  firebase_config.database_url = config.firebase_data_base_url;

  firebase_config.token_status_callback = tokenStatusCallback;

  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  Firebase.begin(&firebase_config, &firebase_auth);
  Firebase.setDoubleDigits(5);
}

String getCurrentTimestamp()
{
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}
int sendDataPrevMillis = 0;
int count = 0;

float readMQ2() {
  int analogValue = analogRead(A0);
  float voltage = analogValue * (5.0 / 1023.0);  // Converte o valor analógico para tensão
  return voltage;  // Ou converte para a unidade desejada, se necessário
}

float readDHT11() {
  float temperature = dht.readTemperature(); // Lê a temperatura em Celsius
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0; // Retorna 0.0 se a leitura falhar
  }
  return temperature;
}

void sendData()
{
  String configName = config.name; // Substitua pelo nome da configuração real
  String basePath = "/" + configName + "/current/";
  String historyPath = "/" + configName + "/history/";

  // Ler os valores dos sensores
  float dhtValue = readDHT11();   // Leitura da temperatura do DHT11
  float mq2Value = readMQ2();     // Leitura do MQ2
  float lumiValue = random(200, 1000) / 10.0; // Exemplo: valores entre 20.0 e 100.0
  float noiseValue = random(500, 1500) / 10.0; // Exemplo: valores entre 50.0 e 150.0
  String ipAddress = WiFi.localIP().toString();
  String wifiSSID = WiFi.SSID();
  String lastUpdate = getCurrentTimestamp();

  // Atualizar os valores mais recentes nos campos específicos
  Serial.printf("Set dht... %s\n", Firebase.setFloat(fbdo, basePath + "dht", dhtValue) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set mq2... %s\n", Firebase.setFloat(fbdo, basePath + "mq2", mq2Value) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set lumi... %s\n", Firebase.setFloat(fbdo, basePath + "lumi", lumiValue) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set noise... %s\n", Firebase.setFloat(fbdo, basePath + "noise", noiseValue) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set ip... %s\n", Firebase.setString(fbdo, basePath + "ip", ipAddress) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set wifi... %s\n", Firebase.setString(fbdo, basePath + "wifi", wifiSSID) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set last_update... %s\n", Firebase.setString(fbdo, basePath + "last_update", lastUpdate) ? "ok" : fbdo.errorReason().c_str());

  // Construir o JSON com os valores atuais e timestamp
  FirebaseJson json;
  json.set("dht", dhtValue);
  json.set("mq2", mq2Value);
  json.set("lumi", lumiValue);
  json.set("noise", noiseValue);
  json.set("ip", ipAddress);
  json.set("wifi", wifiSSID);
  json.set("last_update", lastUpdate);

  // Adicionar o novo registro ao histórico
 Serial.printf("Append to history... %s\n", Firebase.push(fbdo, historyPath, json) ? "ok" : fbdo.errorReason().c_str());

  Serial.println();
}


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
  dht.begin();
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

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    sendData();
  }
}