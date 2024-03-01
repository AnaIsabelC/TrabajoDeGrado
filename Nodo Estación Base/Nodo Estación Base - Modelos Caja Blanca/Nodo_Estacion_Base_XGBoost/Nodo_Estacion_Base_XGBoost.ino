#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "XGBoost.h"

Eloquent::ML::Port::XGBClassifier clf;

const char *ssid = "FAMILIA_CHAVEZ";         // Nombre de la red WiFi
const char *password = "251672000";    // Contraseña de la red WiFi  
const int port = 80;                  // Puerto para la conexión

WiFiServer server(port);

// Variables adicionales para contar lecturas dentro del rango
int tempReadingsInRange = 0;
int humReadingsInRange = 0;
int co2ReadingsInRange = 0;
int tvocReadingsInRange = 0;

float tempTotal = 0;
float humTotal = 0;
int co2Total = 0;
int tvocTotal = 0;
int daysMonitored = 0;
int readings = 0;
float temperatura=0;
float humedad=0;
int co2=0;
int tvco=0;

int intentosConexion = 0;
const int maxIntentos = 10;
const unsigned long tiempoEspera = 5000; // Tiempo de espera en milisegundos entre intentos

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

void sendStartMessageToTelegram() {
  const char* startMessage = "🐝 Hola muy buenos días, el ciclo de monitoreo de 5 días para evaluar el estado de tu colmena ha empezado. Durante estos días se evaluarán las condiciones de las abejas y te enviaremos un informe final. También puedes reiniciar el sistema con el comando '/reiniciar' en caso de que necesites manipular a tus colmenas. Si existe algún problema con el sistema, te notificaremos con un mensaje. ";
  sendTelegramMessage(startMessage);
}

void setup() {
  Serial.begin(9600);
   // Obtener el tamaño total de la memoria flash
    uint32_t flashSize = ESP.getFlashChipSize();
    Serial.print("Tamaño total de la memoria flash: ");
    Serial.println(flashSize);

    // Obtener la cantidad de memoria RAM disponible
    uint32_t freeRAM = ESP.getFreeHeap();
    Serial.print("Memoria RAM disponible: ");
    Serial.println(freeRAM);
    
  WiFi.mode(WIFI_STA);

  int intentosConexion = 0;

  while (WiFi.status() != WL_CONNECTED && intentosConexion < maxIntentos) {
    Serial.print("Intento de conexión #");
    Serial.println(intentosConexion + 1);

    WiFi.begin(ssid, password);

    // Espera hasta que la conexión se establezca o se alcance el tiempo máximo
    unsigned long inicioEspera = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - inicioEspera < tiempoEspera) {
      delay(500); // Puedes ajustar este valor según sea necesario
    }

    intentosConexion++;

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Conexión WiFi establecida. Dirección IP: " + WiFi.localIP().toString());
    } else {
      Serial.println("No se pudo conectar a la red WiFi. Esperando antes del próximo intento...");
      delay(5000); // Puedes ajustar este valor según sea necesario
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Se superó el número máximo de intentos. No se pudo establecer la conexión WiFi.");
    return;
  }

  // Enviar mensaje de inicio a Telegram
  sendStartMessageToTelegram();

  server.begin();
  Serial.println("Servidor iniciado");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Cliente conectado"); 
    while (client.connected()) {
      if (client.available()) {
        String data = client.readStringUntil('\n');
        Serial.println("Datos recibidos: " + data);

        // Split de la cadena por el punto y coma
        String parts[4];
        int index = 0;
        int semicolonIndex = 0;
        int lastIndex = 0;

        while ((semicolonIndex = data.indexOf(';', lastIndex)) != -1 && index < 4) {
          parts[index++] = data.substring(lastIndex, semicolonIndex);
          lastIndex = semicolonIndex + 1;
        }
        // Obtén el último elemento después del último punto y coma
        if (lastIndex < data.length() && index == 3) {
          parts[index] = data.substring(lastIndex);
        }

        // Convertir las partes a valores float e int
        float tempAverage = parts[0].toFloat();
        float humAverage = parts[1].toFloat();
        int co2Average = parts[2].toInt();
        int tvocAverage = parts[3].toInt();

        if (tempAverage >= 19.9 && tempAverage < 37) {
            tempTotal += tempAverage;
            tempReadingsInRange++;
        } else {
            sendTelegramMessage("¡Alerta! Problema con el sensor de temperatura");
        }
        
        if (humAverage >= 35 && humAverage <= 85) {
            humTotal += humAverage;
            humReadingsInRange++;
        } else {
            sendTelegramMessage("¡Alerta! Problema con el sensor de humedad");
        }
        
        if (co2Average >= 400 && co2Average < 3000) {
            co2Total += co2Average;
            co2ReadingsInRange++;
        } else {
            sendTelegramMessage("¡Alerta! Problema con el sensor de CO2");
        }
        
        // Verificación de rangos para TVCO
        if (tvocAverage >= 0 && tvocAverage < 2500) {
            tvocTotal += tvocAverage;
            tvocReadingsInRange++;
        } else {
            sendTelegramMessage("¡Alerta! Problema con el sensor de TVCO");
        }
        
        readings++;

       WiFi.disconnect(); // Desconectar WiFi
      delay(20000); // Esperar 20 segundos (20000 ms)
    
      int intentos = 0;
    
      while (WiFi.status() != WL_CONNECTED && intentos < maxIntentos) {
        Serial.print("Intento de reconexión #");
        Serial.println(intentos + 1);
        
        WiFi.begin(ssid, password);
    
        // Espera hasta que la conexión se establezca o se alcance el tiempo máximo
        unsigned long inicioEspera = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - inicioEspera < tiempoEspera) {
          delay(500); // Puedes ajustar este valor según sea necesario
        }
    
        intentos++;
    
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Conexión WiFi restablecida. Dirección IP: " + WiFi.localIP().toString());
        } else {
          Serial.println("No se pudo reconectar a la red WiFi. Esperando antes del próximo intento...");
          delay(5000); // Puedes ajustar este valor según sea necesario
        }
      }
    
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Se superó el número máximo de intentos. No se pudo restablecer la conexión WiFi.");
        // Puedes tomar acciones adicionales aquí si es necesario
      }
        
        daysMonitored++;
        if (daysMonitored == 5) {
          // Calcular promedios
          tempTotal /= tempReadingsInRange;
          humTotal /= humReadingsInRange;
          co2Total /= co2ReadingsInRange;
          tvocTotal /= tvocReadingsInRange;

          temperatura=tempTotal;
          humedad=humTotal;
          co2=co2Total;
          tvco=tvocTotal;
          
        // Verificación de rangos
        if (temperatura >= 20 && temperatura <= 37) {
          Serial.println("Temperatura dentro del rango seguro");
        } else {
          Serial.println("¡Alerta! Temperatura fuera del rango seguro");
        }

        if (humedad >= 35 && humedad <= 85) {
          Serial.println("Humedad dentro del rango seguro");
        } else {
          Serial.println("¡Alerta! Humedad fuera del rango seguro");
        }

        if (co2 >= 400 && co2 <= 3000) {
          Serial.println("CO2 dentro del rango seguro");
        } else {
          Serial.println("¡Alerta! CO2 fuera del rango seguro");
        }

        if (tvco >= 0 && tvco <= 2500) {
          Serial.println("TVCO dentro del rango seguro");
        } else {
          Serial.println("¡Alerta! TVCO fuera del rango seguro");
        }
        
        // Aquí puedes utilizar los valores procesados como necesites
        Serial.print("Temperatura: ");
        Serial.println(temperatura);
        Serial.print("Humedad: ");
        Serial.println(humedad);
        Serial.print("CO2: ");
        Serial.println(co2);
        Serial.print("TVCO: ");
        Serial.println(tvco);
        
        // Variables para almacenar el peso de cada nivel de alerta
        int pesoTemperatura = 0;
        int pesoHumedad = 0;
        int pesoCO2 = 0;
        int pesoTVCO = 0;

        // Definiciones de valores y cálculos
        float valor_maximo_temp = 37;
        float valor_minimo_temp = 20;
        float recorrido_temp = valor_maximo_temp - valor_minimo_temp;
        float peso_temp = 0.48;
        
        float valor_maximo_hum = 85;
        float valor_minimo_hum = 35;
        float recorrido_hum = valor_maximo_hum - valor_minimo_hum;
        float peso_hum = 0.24;
        
        int valor_maximo_co2 = 3000;
        int valor_minimo_co2 = 400;
        int recorrido_co2 = valor_maximo_co2 - valor_minimo_co2;
        float peso_co2 = 0.16;
        
        int valor_maximo_tvoc = 2500;
        int valor_minimo_tvoc = 0;
        int recorrido_tvoc = valor_maximo_tvoc - valor_minimo_tvoc;
        float peso_tvoc = 0.12;

        // Definiciones de funciones de asignación de puntuaciones
        float asignar_puntuacion_temperatura= (valor_maximo_temp - temperatura) / recorrido_temp;        
        float asignar_puntuacion_humedad =(humedad - valor_minimo_hum) / recorrido_hum;      
        float asignar_puntuacion_co2=(co2 - valor_minimo_co2) / recorrido_co2;        
        float asignar_puntuacion_tvco= (tvco - valor_minimo_tvoc) / recorrido_tvoc;
        

        float peso_temperatura = asignar_puntuacion_temperatura * peso_temp;
        float peso_humedad = asignar_puntuacion_humedad * peso_hum;
        float peso_Co2 = asignar_puntuacion_co2 * peso_co2;  
        float peso_Tvco = asignar_puntuacion_tvco * peso_tvoc; 

        // Calculando la suma de los pesos
        float suma_pesos = peso_temperatura + peso_humedad + peso_Co2 + peso_Tvco;
        
        Serial.print("Suma ");
        Serial.println(suma_pesos,5);
        
        String nivel_alerta;

        float AlertSample[4] =  {peso_temperatura,peso_humedad,peso_Co2,peso_Tvco}; 

        Serial.print("Predicción: ");
        Serial.println(clf.predict(AlertSample),5);
        int Alerta= 0;
       
        Alerta= clf.predict(AlertSample);
        
        Serial.print(Alerta);
            
        // Después de calcular el nivel de alerta y la suma de pesos
        
        if (Alerta == 3) {
            sendTelegramMessage("Nivel general de alerta: MUY ALTO        ¡Alerta! Parece que tus abejas están alcanzando un nivel muy alto de infestación por varroa. Es necesario tomar medidas cuanto antes para evitar daños irreversibles");
            sendTelegramMessage("1) Verifica la ubicación de tu colmena, probablemente no esté localizada en un lugar adecuado. Recuerda que debe estar donde reciba la luz del sol, no tenga exceso de humedad, que esté alejada del suelo y lo principal, comprobar la presencia de otros colmenares vecinos e investigar sobre su situación");
            sendTelegramMessage("2) Realiza una limpieza general de la colmena. la limpieza mediante el uso de soda cáustica, ácido oxálico o agua oxigenada. Al desinfectar los cuadros, se eliminan residuos de cera, propóleos, polen y posibles ácaros adheridos");
            sendTelegramMessage("3) Utiliza un tratamiento para frenar su avance,si no deseas usar productos químicos  La aplicación aceites esenciales como lavanda y laurel contribuyen al desprendimiento efectivo de los ácaros desde las abejas, provocando una caída natural de varroa. Es un tratamiento natural y que contribuye a la prevención del ácaro ");
            sendTelegramMessage("4) Tratamientos que pueden ser efectivos mediante el uso de soda cáustica, ácido oxálico o agua oxigenada. Al desinfectar los cuadros, se eliminan residuos de cera, propóleos, polen y posibles ácaros adheridos. ");
        } else if (Alerta ==0 ) {
            sendTelegramMessage("Nivel general de alerta: ALTO            ¡Cuidado! Parace ser que la varroa ya ha entrado a tu colmena y se encuentra en una fase de desarrollo bastante considerable. Para esto te hacemos las siguientes recomendaciones:");
            sendTelegramMessage("1) Verifica la ubicación de tu colmena, probablemente no esté localizada en un lugar adecuado. Recuerda que debe estar donde reciba la luz del sol, no tenga exceso de humedad, que esté alejada del suelo y lo principal, comprobar la presencia de otros colmenares vecinos e investigar sobre su situación");
            sendTelegramMessage("2) Realiza una limpieza general de la colmena. la limpieza mediante el uso de soda cáustica, ácido oxálico o agua oxigenada. Al desinfectar los cuadros, se eliminan residuos de cera, propóleos, polen y posibles ácaros adheridos");
            sendTelegramMessage("3) Utiliza un tratamiento para frenar su avance,si no deseas usar productos químicos  La aplicación aceites esenciales como lavanda y laurel contribuyen al desprendimiento efectivo de los ácaros desde las abejas, provocando una caída natural de varroa. Es un tratamiento natural y que contribuye a la prevención del ácaro ");
        } else if (Alerta == 2) {
            sendTelegramMessage("Nivel general de alerta: MEDIO           ¡Bien! Tu colmena parece estar sana. Sin embargo, es posible que tengas un porcentaje de infestación de varroa medio. Esto significa que probvablemente el ácaro esté tomando fuerza al interior de tu colmena. Para esto, te hacemos las siguientes recomendaciones  ");
            sendTelegramMessage("1) Para tu siguiente visita al apiario, recuerda hacer una limpieza de la colmena. Elimina la cera que se encuentre en mal estado y retira el exceso de humedad");
            sendTelegramMessage("2) Recuerda que la colmena debe estar localizada en un lugar adecuado, donde reciba la luz del sol, no tenga exceso de humedad y que esté alejada del suelo");
            sendTelegramMessage("3) Si lo deseas, La aplicación aceites esenciales como lavanda y laurel contribuyen al desprendimiento efectivo de los ácaros desde las abejas, provocando una caída natural de varroa. Es un tratamiento natural y que contribuye a la prevención del ácaro ");
        } else if (Alerta == 1) {
            sendTelegramMessage("Nivel general de alerta: BAJO           ¡Felicitaciones! Tu colmena actualemente se encuentra bastante sana. Tus abejas están en su máximo nivel de producción y hay pocas probabilidades de presencia de varroa. Recuerda seguir las recomendaciones para que continuen sanas");
            
        } else {
            sendTelegramMessage("Nivel general de alerta: Fuera de rango");
        } 
        
      }
        }
      }
    Serial.println("Cliente desconectado");
    client.stop();
    }
}
