#include <Adafruit_SGP30.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_sleep.h>


#define DHTPIN 4      // Pin del sensor DHT
#define DHTTYPE DHT22 // Tipo de sensor DHT (en este caso DHT22)

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SGP30 sgp;

int dhtReadings = 0;
int sgpReadings = 0;
int cycleCount = 0;

float tempTotal = 0;
float humTotal = 0;
int co2Total = 0;
int tvocTotal = 0;


const int batteryPin = 34; // Pin analógico al que está conectada la batería
const char *ssid = "CAICEDO";         // Nombre de la red WiFi
const char *password = "10N56c14";    // Contraseña de la red WiFi
const char *host = "192.168.0.8";   // Dirección IP del ESP32 destino  192.168.135.109
const int port = 80;                  // Puerto para la conexión

WiFiClient client;


bool sendDataOverWiFi_Temp_Hum(float temp, float hum) {
  if (client.connect(host, port)) {
    Serial.println("Conexión establecida con el servidor");
    String data = String(temp) + ";" + String(hum) + "\r\n";
    client.println(data);
    delay(4000);  // Espera 1 segundo antes de cerrar la conexión
    client.stop();
    Serial.println("Datos enviados por WiFi");
    return true;
  } else {
    Serial.println("Fallo en la conexión WiFi");
    return false;
  }
}

bool sendDataOverWiFi_CO2_TVOC(int co2, int tvoc) {
  if (client.connect(host, port)) {
    Serial.println("Conexión establecida con el servidor");
    String data = String(co2) + ";" + String(tvoc) + "\r\n";
    client.println(data);
    delay(4000);  // Espera 1 segundo antes de cerrar la conexión
    client.stop();
    Serial.println("Datos enviados por WiFi");
    return true;
  } else {
    Serial.println("Fallo en la conexión WiFi");
    return false;
  }
}

bool connectToWiFi() {
  int intentosConexion = 0;
  const int maxIntentosConexion = 5; // Número máximo de intentos

    while (intentosConexion < maxIntentosConexion) {
    WiFi.begin(ssid, password);
    Serial.println("Intentando conectar a la red WiFi...");

    int intento = 0;
    while (intento < 20 && WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      intento++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("\nConexión WiFi establecida. Dirección IP: ");
      Serial.println(WiFi.localIP());
      return true;
    } else {
      Serial.println("\nFallo al conectar a la red WiFi. Reintentando...");
      intentosConexion++;
      delay(1000); // Espera antes de intentar nuevamente
    }
  }

  Serial.println("No se pudo conectar a la red WiFi después de varios intentos.");
  return false;
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  if (!sgp.begin()){
    Serial.println("No se pudo iniciar el sensor SGP30. Verifique las conexiones.");
    while (1);
  }
  
  if (!sgp.IAQinit()) {
    Serial.println("Inicialización de IAQ fallida. Verifique las conexiones.");
    while (1);
  }
  Serial.println("Sensores inicializados correctamente.");

}

void loop() {
  
}
