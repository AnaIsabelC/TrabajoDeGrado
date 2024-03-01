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
const char *ssid = "iPhone de Ana";         // Nombre de la red WiFi
const char *password = "@ana1234";    // Contraseña de la red WiFi
const char *host = "172.20.10.8";   // Dirección IP del ESP32 destino  192.168.135.109
const int port = 80;                  // Puerto para la conexión

const char* BOT_TOKEN = "6974862396:AAHI33xvfFhQ6EVmW0L7X6ZG9cncgCcBGlg";
const char* CHAT_ID = "1568816699"; // Puedes obtener este valor al iniciar una conversación con el bot @userinfobot

void sendTelegramMessage(const char* message) {
  const int maxRetries = 3;  // Número máximo de intentos
  int currentRetry = 0;

  while (currentRetry < maxRetries) {
    HTTPClient http;
    http.begin("https://api.telegram.org/bot" + String(BOT_TOKEN) + "/sendMessage");
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["chat_id"] = CHAT_ID;
    doc["text"] = message;

    String jsonStr;
    serializeJson(doc, jsonStr);

    int httpResponseCode = http.POST(jsonStr);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
      http.end();
      return;  // Salir del bucle si la solicitud fue exitosa
    } else {
      Serial.println("Error en la solicitud. Intento #" + String(currentRetry + 1));
      currentRetry++;
    }

    http.end();
    delay(1000);  // Esperar un segundo entre intentos (puedes ajustar este valor)
  }

  Serial.println("Se superó el número máximo de intentos. Error en la solicitud.");
}


WiFiClient client;

float measureBatteryVoltage() {
  // Supongamos que el sensor FZ0430 está conectado al pin analógico batteryPin
  int sensorValue = analogRead(batteryPin);
  
  // Ajusta esta lógica según las especificaciones del sensor FZ0430
   float voltaje = sensorValue * (5.0 / 1023.0); // En este caso, se asume una referencia de voltaje de 5V // Suponiendo que el sensor FZ0430 da una salida entre 0 y 5 V

    return voltaje;
}

bool sendDataOverWiFi(float tempAverage, float humAverage, int co2Average, int tvocAverage) {
  if (client.connect(host, port)) {
    Serial.println("Conexión establecida con el servidor");
    String data = String(tempAverage) + ";" + String(humAverage) + ";" + String(co2Average) + ";" + String(tvocAverage) + "\r\n";
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
// Medir el nivel de la batería al inicio del ciclo
  float batteryVoltage = measureBatteryVoltage();
  Serial.print("Voltaje de la batería al inicio del ciclo: ");
  Serial.print(batteryVoltage);
  Serial.println(" V");

  // Conectar a la red WiFi
  bool wifiConnected = connectToWiFi();

  // Enviar mensaje de Telegram con el nivel de voltaje
  if (wifiConnected) {
   String message = "La batería del Nodo al incio del ciclo es de " + String(batteryVoltage) + " V";
    sendTelegramMessage(message.c_str()); // Convertir a const char* antes de enviar
    // Desconectar WiFi después de enviar el mensaje
    WiFi.disconnect(true);
    Serial.println("Desconectado de la red WiFi");
  }
}


void loop() {
 static bool measurementsAdjusted = false;
  static int dhtReadingsTarget = 6;
  static int sgpReadingsTarget = 20;
  static int daysMonitored = 0;
  static int dhtDelay = 0; // 2 segundos en milisegundos
  static int sgpDelay = 0; // 1 segundo en milisegundos

  // Medición del nivel de la batería y ajuste de las lecturas
  if (!measurementsAdjusted) {
    float batteryVoltage = measureBatteryVoltage();
    
    Serial.print("Voltaje de la batería: ");
    Serial.print(batteryVoltage);
    Serial.println(" V");

    // Ajustar las lecturas según el voltaje de la batería
    if (batteryVoltage < 5.5) {
      dhtReadingsTarget = 3;
      sgpReadingsTarget = 10;
      // Si el voltaje es bajo, ajustar los retrasos
      dhtDelay = 4000; // 4 segundos en milisegundos
      sgpDelay = 2000; // 2 segundos en milisegundos
    } else {
      dhtReadingsTarget = 6;
      sgpReadingsTarget = 20;
      // Si el voltaje es normal, establecer los retrasos normales
      dhtDelay = 2000; // 2 segundos en milisegundos
      sgpDelay = 2000; // 1 segundo en milisegundos
    }

    measurementsAdjusted = true; // Indicar que las mediciones han sido ajustadas
  }

  delay(1000); // Puedes ajustar el intervalo de lectura según tus necesidades
  if (cycleCount < 2) {
    if (dhtReadings < dhtReadingsTarget) {
     
      float tempDHT = dht.readTemperature(); // Lectura de la temperatura
      float humDHT = dht.readHumidity();    // Lectura de la humedad
      
      if (!isnan(tempDHT) && !isnan(humDHT)) {
        Serial.print("Temperatura DHT: ");
        Serial.print(tempDHT);
        Serial.println(" °C");
        Serial.print("Humedad DHT: ");
        Serial.print(humDHT);
        Serial.println(" %");
        tempTotal += tempDHT;
        humTotal += humDHT;
        dhtReadings++;
      } else {
        Serial.println("Fallo al leer el sensor DHT. Reintentando...");
      }
       // Configurar el tiempo de espera antes de la siguiente medición
      esp_sleep_enable_timer_wakeup(dhtDelay * 1000);  // Convertir el tiempo a microsegundos

      // Entrar en el modo de bajo consumo
      esp_light_sleep_start();
      
    } else if (sgpReadings < sgpReadingsTarget) {
      if (sgp.IAQmeasure()) {
        Serial.print("CO2 equiv: ");
        Serial.print(sgp.eCO2);
        Serial.println(" ppm");
        Serial.print("TVOC: ");
        Serial.print(sgp.TVOC);
        Serial.println(" ppb");
        co2Total += sgp.eCO2;
        tvocTotal += sgp.TVOC;
        sgpReadings++;
      } else {
        Serial.println("Fallo al medir la calidad del aire. Reintentando...");
      }
      delay(sgpDelay); // Espera 1 segundo entre cada lectura de SGP30
    } else {
      dhtReadings = 0;
      sgpReadings = 0;
      cycleCount++;
    }
  } else {
    
    float tempAverage = tempTotal / (float)(dhtReadingsTarget * 2);
    float humAverage = humTotal / (float)(dhtReadingsTarget * 2);
    int co2Average = co2Total / (sgpReadingsTarget * 2);
    int tvocAverage = tvocTotal / (sgpReadingsTarget * 2);

    Serial.println("Promedios:");
    Serial.print("Temperatura promedio: ");
    Serial.print(tempAverage);
    Serial.println(" °C");
    Serial.print("Humedad promedio: ");
    Serial.print(humAverage);
    Serial.println(" %");
    Serial.print("CO2 promedio: ");
    Serial.print(co2Average);
    Serial.println(" ppm");
    Serial.print("TVOC promedio: ");
    Serial.print(tvocAverage);
    Serial.println(" ppb");
    
// Conexión a la red WiFi
bool wifiConnected = false;
int intentosConexion = 0;
const int maxIntentosConexion = 5; // Número máximo de intentos

while (!wifiConnected && intentosConexion < maxIntentosConexion) {
  WiFi.begin(ssid, password);
  Serial.println("Intentando conectar a la red WiFi...");

  int intento = 0;
  while (intento < 20 && WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    intento++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.print("\nConexión WiFi establecida. Dirección IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFallo al conectar a la red WiFi. Reintentando...");
    intentosConexion++;
    delay(1000); // Espera antes de intentar nuevamente
  }
}

if (wifiConnected) {
  sendDataOverWiFi(tempAverage, humAverage, co2Average, tvocAverage);
  
  // Después de enviar los datos, desconectar WiFi para ahorrar energía
  WiFi.disconnect(true);
  Serial.println("Desconectado de la red WiFi");
  daysMonitored++;
} else {
  Serial.println("No se pudo conectar a la red WiFi después de varios intentos.");
  // Aquí podrías añadir algún otro comportamiento, como reiniciar el dispositivo o tomar otra acción.
}
    
    if (daysMonitored < 5) {
      // Después de enviar los datos de cada ciclo de 5 días...
      float batteryVoltage = measureBatteryVoltage();

      Serial.print("Nivel de batería al final del ciclo: ");
      Serial.print(batteryVoltage);
      Serial.println(" V");

      // Ajustar las lecturas según el voltaje de la batería medido al final del ciclo
      if (batteryVoltage < 5.5) {
        dhtReadingsTarget = 3;
        sgpReadingsTarget = 10;
      } else {
        dhtReadingsTarget = 6;
        sgpReadingsTarget = 20;
      }

      // Reiniciar contadores para iniciar un nuevo ciclo
      dhtReadings = 0;
      sgpReadings = 0;
      cycleCount = 0;
      tempTotal = 0;
      humTotal = 0;
      co2Total = 0;
      tvocTotal = 0;

      Serial.println("Datos enviados al final del día");
    } else {
      // Si se completan los 5 días, se envían los datos y se detiene el monitoreo
      while (true) {} 
    }
  }
}
