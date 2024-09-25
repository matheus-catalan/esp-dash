#include "HTTPServer.h"
#include "ConfigManager.h"
#include "WifiManager.h"
//#include <ESPAsyncWebServer.h>
//#include <AsyncTCP.h>

#if defined(ESP8266)
  ESP8266WebServer server(80);
#else
  WebServer server(80);
  //AsyncWebServer serverAsync(81);

#endif

extern Config config;

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

      h3 {
        text-align: center;
        margin-bottom: 23px;
        margin-top: 0;
        color: #D9D9D9;
        font-size: 20px;
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

      toast {
        position: fixed;
        bottom: 20px;
        right: 20px;
        min-width: 250px;
        margin: auto;
        padding: 15px;
        border-radius: 5px;
        color: #fff;
        background-color: #333;
        text-align: center;
        font-size: 14px;
        box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
        opacity: 0;
        transition: opacity 0.5s ease-in-out;
      }

      .toast.show {
        opacity: 1;
      }

      .toast.success {
        background-color: #4CAF50; /* Green */
      }

      .toast.error {
        background-color: #f44336; /* Red */
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

        window.location.href = 'http://192.168.1.1/';
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
    <div id="toast" class="toast hidden"></div>
    <div class="container">
      <div class="section" style="display: flex; align-items: center; justify-content: center;" >
        <h2 style="color: #fff; font-size: 30px;"> name1_env </h2>
      </div>
      <div class="section" style="display: flex; align-items: center; justify-content: center;">
        <form action='/action_form_restart' method='post'>
          <button class="button"
                    style="color: #000; background-color: #D9D9D9; font-size: 15px; height: 30px; max-width: 200px; border: 1px solid #B0B0B0;">Reiniciar dispostivo</button>
        </form>
      </div>
      <div class="section">
        <h4> Configuração Wi-Fi</h4>
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
        <h4>Configurações Dispositivo</h4>
        <form action='/action_form_credentials' method='post'>
          <div class="field-group">
            <span style="color: #D9D9D9; margin-bottom: 5px;">Nome do dispositivo</span>
            name2_input
          </div>
          <div class="field-group">
            <span style="color: #D9D9D9; margin-bottom: 5px;">Url do servidor</span>
            base_url_input
          </div>
          <div class="field-group">
            <span style="color: #D9D9D9; margin-bottom: 5px;">MQTT</span>
            mqtt_url_input
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

String listSSID()
{
  String index = (const __FlashStringHelper *)MAIN_page;
  String networks = "";
  int n = WiFi.scanNetworks();

  if (n > 0) {
    networks = "<tbody>";
    for (int i = 0; i < n; ++i) {
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
      networks += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Aberta" : "Fechada");
      networks += "</td>";
      networks += "<td style='text-align: right;'><button  type='button' class='button button-";
      if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
        networks += "connected' onclick=\"toggleDisconnect(this, '";
        networks += ssid;
        networks += "')\">Desconectar";
      } else {
        networks += "-to-connected' onclick=\"toggleConnection(this, '";
        networks += ssid;
        networks += "')\">Conectar";
      }
      networks += "</button></td></tr>";
    }
    networks += "</tbody>";

    index.replace("<tbody><tr><td colspan=\"5\"> Nenhuma rede disponível</td></tr></tbody>", networks);
  }

  
  return index;
}

String setConfig(String index) {
  index.replace("name1_env", String(config.name));
  index.replace("name2_input", "<input class='text-field' type='text' name='name' maxlength='32' placeholder='Name' value='" + String(config.name) + "'>");
  index.replace("base_url_input", "<input class='text-field' type='text' name='base_url' maxlength='64' placeholder='Server url' value='" + String(config.base_url) + "'>");
  index.replace(
  "mqtt_url_input",
    "<div style='display: flex; align-items: center; gap: 10px;'> \
     <input class='text-field' type='text' name='base_url' maxlength='64' placeholder='MQTT url' value='" + String(config.mqtt_url) + "' style='flex: 2;'> \
     <input class='text-field' type='text' name='mqtt_port' maxlength='5' placeholder='Port' value='" + String(config.mqtt_port) + "' style='flex: 1;'> \
    </div>"
  );
  return index;
}

String handleIndex() {
  String index = (const __FlashStringHelper *)MAIN_page;
  index = listSSID();
  index = setConfig(index);
  return index;
}

void handleRoot() {
  server.send(200, "text/html", handleIndex());
}

void handleFormConnection() {
  String ssidWifi = server.arg("ssid");
  String passwordWifi = server.arg("password");
  String index = (const __FlashStringHelper *)MAIN_page;
  
  if (!ssidWifi.isEmpty() && !passwordWifi.isEmpty()) {
    connectToWiFi(ssidWifi, passwordWifi);
    //index.replace("<p class='status'>status_wifi</p>", "<p>Conectando ao Wi-Fi...</p>");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    saveConfig(config);
    //index.replace("<p>Conectado ao Wi-Fi...</p>", "<p>Conectado ao Wi-Fi</p>");
  } else {
    //index.replace("<p>Conectando ao Wi-Fi..</p>", "<p>Falha ao conectar ao Wi-Fi</p>");
  }
  handleRoot();
}

void handleClient() {
  server.handleClient();
}

void handleFormCredentials()
{
  String name = server.arg("name");
  String base_url = server.arg("base_url");
  
  strncpy(config.name, name.c_str(), sizeof(config.name) - 1);
  strncpy(config.base_url, base_url.c_str(), sizeof(config.base_url) - 1);
  strncpy(config.mqtt_url, server.arg("mqtt_url").c_str(), sizeof(config.mqtt_url) - 1);
  config.mqtt_port = server.arg("mqtt_port").toInt();
  saveConfig(config);

  handleRoot();
}

/*void update_config(AsyncWebServerRequest *request) {
  if (request->hasParam("sensor_key", true) && request->hasParam("min_value", true)) {
    String sensorKey = request->getParam("sensor_key", true)->value();
    String minValue = request->getParam("min_value", true)->value();
    String maxValue = request->getParam("max_value", true)->value();
    String alertNotification = request->getParam("alert_notification", true)->value();
    String alertLight = request->getParam("alert_light", true)->value();
    String alertSound = request->getParam("alert_sound", true)->value();


    request->send(200, "text/plain", "Configuração salva!");
  } else {
    request->send(400, "text/plain", "Parâmetros ausentes!");
  }
}*/

void setupHttpServer() {
  Serial.println("\n=========================== HTTP SERVER ===========================");
  Serial.println("Iniciando HTTP server...");
  server.on("/", HTTP_GET, handleRoot);
  server.on("/action_new_connection", handleFormConnection);
  server.on("/action_form_credentials", handleFormCredentials);
  server.on("/action_form_restart", HTTP_POST, []() {
    server.send(200, "text/plain", "Reiniciando dispositivo...");
    delay(1000);
    ESP.restart();
  });
  
  server.begin();

  Serial.println("HTTP server iniciado");
  Serial.println("Accesse http://" + WiFi.localIP().toString() + " para configurar o dispositivo");
  Serial.println("=========================== HTTP SERVER ===========================");
}