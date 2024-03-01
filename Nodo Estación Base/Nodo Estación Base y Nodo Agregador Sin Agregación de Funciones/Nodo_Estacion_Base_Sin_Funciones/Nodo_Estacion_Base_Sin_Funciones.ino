#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char *ssid = "FAMILIA_CHAVEZ";         // Nombre de la red WiFi
const char *password = "251672000";    // Contraseña de la red WiFi
const int port = 80;
const int maxReconnectAttempts = 10;  // Número máximo de intentos de reconexión

WiFiServer server(port);

void setup() {
  Serial.begin(9600);

  // Conectar a la red WiFi
  WiFi.begin(ssid, password);
  int reconnectAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && reconnectAttempts < maxReconnectAttempts) {
    delay(2000);
    Serial.println("Conectando a WiFi...");
    reconnectAttempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexión WiFi establecida");
    Serial.println(WiFi.localIP());

    // Inicializar el servidor
    server.begin();
    Serial.println("Servidor iniciado");
  } else {
    Serial.println("Fallo al conectar a WiFi. Reinicie el dispositivo.");
    while (true) {}
  }
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Cliente conectado");

    while (client.connected()) {
      if (client.available()) {
        // Leer datos del cliente
        String data = client.readStringUntil('\r');
        Serial.println("Datos recibidos: " + data);
        // Desconectar de la red WiFi después de procesar los datos
        WiFi.disconnect();
        delay(2000);  // Espera 2 segundos antes de volver a conectarse

        // Volver a conectarse a la red WiFi
        WiFi.begin(ssid, password);
        int reconnectAttempts = 0;
        while (WiFi.status() != WL_CONNECTED && reconnectAttempts < maxReconnectAttempts) {
          delay(2000);
          Serial.println("Reconectando a WiFi...");
          reconnectAttempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Reconexión WiFi establecida");
        } else {
          Serial.println("Fallo al reconectar a WiFi. Reinicie el dispositivo.");
          while (true) {}
        }
      }
    }

    Serial.println("Cliente desconectado");
    client.stop();
  }
}
