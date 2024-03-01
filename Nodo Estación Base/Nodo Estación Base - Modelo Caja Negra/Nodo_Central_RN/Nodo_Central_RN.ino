// NODO ESTACIÓN BASE

// Inclusión de Librerías 
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "RedNeuronal.h"
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>

// Creación y Declaración del modelo
#define ARENA_SIZE 5000
Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;


// Constantes Para conexión a Internet
const char *ssid = "CAICEDO";         // Nombre de la red WiFi
const char *password = "10N56c14";    // Contraseña de la red WiFi
const int port = 80;                  // Puerto para la conexión
WiFiServer server(port);


// Variables para la recepción de datos 
float temperatura=0;
float humedad=0;
int co2=0;
int tvco=0;


// Variables adicionales para el conteo de lecturas dentro del rango adecuado
int tempReadingsInRange = 0;
int humReadingsInRange = 0;
int co2ReadingsInRange = 0;
int tvocReadingsInRange = 0;


// Declaración de variables para almacenamiento de los datos
float tempTotal = 0;
float humTotal = 0;
int co2Total = 0;
int tvocTotal = 0;

//Variable para el conteo de días de monitoreo
int daysMonitored = 0;

//Variable para el conteo de lecturas en rangos válidos
int readings = 0;

//Variable para los intentos de conexión
int intentosConexion = 0;
//Constantes para establecer máximo de intentos y tiempo de espera
const int maxIntentos = 10;
const unsigned long tiempoEspera = 5000; // Tiempo de espera en milisegundos entre intentos


// Constantes para Acceder al bot de Telegram
const char* BOT_TOKEN = "6974862396:AAHI33xvfFhQ6EVmW0L7X6ZG9cncgCcBGlg";
const char* CHAT_ID = "1568816699"; 


//Función Para el envío de mensajes vía Telegram
void sendTelegramMessage(const char* message) {
  const int maxRetries = 5;  // Número máximo de intentos
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

//Mensaje de Inicialización del Sistema
void sendStartMessageToTelegram() {
  const char* startMessage = "🐝 Muy buenos días, el ciclo de monitoreo de 5 días para evaluar el estado de tu colmena ha empezado. "
                             "Durante estos días se evaluarán las condiciones de tus abejas y te enviaremos un informe final, que te dirá cuál es el nivel de alerta de presencia de varroa.🚦🕷️ "
                             "Para tu tranquilidad, si en alguno de los días de monitoreo se presenta una anomalía, o condición desfavorable en tú colmena, te lo haremos saber.⚠️ "
                             "De igual forma, si existe algún problema con el sistema o sus componentes, te notificaremos con un mensaje. Que tengas buen día!! 😊";
  sendTelegramMessage(startMessage);
}


// Configuración Inicial del Sistema
void setup() {
  Serial.begin(9600);

  // Establecer Número de Entradas y Salidas del Modelo 
  tf.setNumInputs(4);
  tf.setNumOutputs(4);
  tf.resolver.AddFullyConnected();
  tf.resolver.AddSoftmax();

    while (!tf.begin(RedNeuronal).isOk()) 
        Serial.println(tf.exception.toString());
        
  //Inicialización de la conexión Vía Wifi      
  WiFi.mode(WIFI_STA);

  int intentosConexion = 0;

  while (WiFi.status() != WL_CONNECTED && intentosConexion < maxIntentos) {
    Serial.print("Intento de conexión #");
    Serial.println(intentosConexion + 1);

    WiFi.begin(ssid, password);

    // Espera hasta que la conexión se establezca o se alcance el tiempo máximo
    unsigned long inicioEspera = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - inicioEspera < tiempoEspera) {
      delay(1000); 
    }

    intentosConexion++;

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Conexión WiFi establecida. Dirección IP: " + WiFi.localIP().toString());
    } else {
      Serial.println("No se pudo conectar a la red WiFi. Esperando antes del próximo intento...");
      delay(5000);
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
}


// Bucle Principal del programa
void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Cliente conectado"); 
    while (client.connected()) {
      if (client.available()) {
        String data = client.readStringUntil('\n');
        Serial.println("Datos recibidos: " + data);

        // Split de la cadena por el punto y coma (Mensajes recibidos del Nodo Agregador)
        String parts[4];
        int index = 0;
        int semicolonIndex = 0;
        int lastIndex = 0;

        while ((semicolonIndex = data.indexOf(';', lastIndex)) != -1 && index < 4) {
          parts[index++] = data.substring(lastIndex, semicolonIndex);
          lastIndex = semicolonIndex + 1;
        }
        // Obtener el último elemento después del último punto y coma
        if (lastIndex < data.length() && index == 3) {
          parts[index] = data.substring(lastIndex);
        }

        // Convertir las partes a valores float e int
        float tempAverage = parts[0].toFloat();
         // Mensajes de Alerta para el usuario
        if (tempAverage >= 19.9 && tempAverage < 37) {
            tempTotal += tempAverage;
            tempReadingsInRange++;
        } else if (tempAverage < 19.9) {
          sendTelegramMessage("¡Alerta! La temperatura dentro de tu colmena es demasiado baja.🥶 "
                            "Durante el último día de monitoreo, hemos registrado que tu colmena presenta temperaturas que están por debajo del rango seguro.⚠️"
                            "Como sabes, esto favorese la proliferación de la Varroa 🕷️. Para eso, te hacemos las siguientes recomendaciones");
          sendTelegramMessage("1) Verifica que las entradas de la colmena no esté bloqueada por obstáculos que impidan que las abejas entren o salgan fácilmente, y tengan la ventilación adecuada");                 
          sendTelegramMessage("2) Si es posible, agrega aislamiento adicional alrededor de la colmena para ayudar a retener el calor. Puedes usar materiales como paja, cartón o placas de poliestireno. (Icopor)");                 
          sendTelegramMessage("3) Asegúrate de que las abejas tengan suficiente suministro de alimentos, como miel y polen, para mantenerse nutridas durante el clima frío.");                 
        } else {
          sendTelegramMessage("¡Alerta! La temperatura dentro de tu colmena es demasiado alta 🥵"
                              "Durante el último día de monitoreo, hemos registrado que tu colmena presenta temperaturas que están por encima del rango seguro.⚠️"
                              "Como sabes, esto afecta directamente la salud de tus abejas🐝 causándoles estrés y baja productividad. Para eso, te hacemos las siguientes recomendaciones");
          sendTelegramMessage("1) Proporciona sombra. Si es posible, coloca la colmena en un lugar sombreado para ayudar a reducir la exposición directa al sol. También puedes instalar estructuras como toldos o sombrillas para crear sombra adicional.");                 
          sendTelegramMessage("2) Verifica de que la colmena tenga una buena ventilación para permitir que el aire circule y se disipe el calor. Puedes abrir la entrada de la colmena para permitir un mejor flujo de aire o agregar ventilación adicional como rejillas de ventilación.");                 
          sendTelegramMessage("3) Asegúrate de que las abejas tengan suficiente suministro de alimentos, como miel y polen, para mantenerse nutridas en caso de que las altas temperaturas no les permitan salir.");                 
          sendTelegramMessage("4) Proporciona agua fresca. Coloca fuentes de agua fresca cerca de la colmena para que las abejas puedan beber y refrescarse.");                 
        
        }
        float humAverage = parts[1].toFloat();

        if (humAverage >= 35 && humAverage <= 85) {
            humTotal += humAverage;
            humReadingsInRange++;
        } else if (humAverage < 35) {
            sendTelegramMessage("¡Alerta! La humedad dentro de tu colmena es demasiado baja. 🏜️"
                                "Durante el último día de monitoreo, hemos registrado que tu colmena presenta niveles de humedad que están por debajo del rango seguro.⚠️"
                                "Como sabes, esto afecta directamente la salud de tus abejas🐝 causándoles estrés y baja productividad. Para eso, te hacemos las siguientes recomendaciones");
            sendTelegramMessage("1) Proporciona fuentes de agua. Coloca recipientes con agua dentro o cerca de la colmena para que las abejas puedan acceder fácilmente y transportarla dentro de la colmena.");                 
            sendTelegramMessage("2) Verifica de que la colmena tenga una buena ventilación para permitir que el aire circule y se disipe el calor. Puedes abrir la entrada de la colmena para permitir un mejor flujo de aire o agregar ventilación adicional como rejillas de ventilación.");                 
            sendTelegramMessage("3) Rocía agua alrededor de la colmena: Rocía agua alrededor de la colmena para aumentar la humedad en el aire circundante. Evita rociar directamente sobre las abejas para no perturbarlas.");                 
            
        } else {
            sendTelegramMessage("¡Alerta! La humedad dentro de tu colmena es demasiado alta.💦"
                                "Durante el último día de monitoreo, hemos registrado que tu colmena presenta niveles de humedad que están por encima del rango seguro.⚠️");
            sendTelegramMessage("1) Asegúrate de que la colmena tenga una buena ventilación. Esto permite que el aire fresco circule y ayuda a reducir la humedad. Puedes considerar agregar ventilaciones en la parte superior o inferior de la colmena para facilitar el flujo de aire.");                 
            sendTelegramMessage("2) La condensación puede aumentar la humedad dentro de la colmena. Asegúrate de que la tapa de la colmena esté bien aislada y que no haya fugas de agua. También puedes utilizar materiales absorbentes, como virutas de madera o tela de fieltro, en el techo de la colmena para absorber el exceso de humedad");                 
            sendTelegramMessage("3) Si la colmena está expuesta a la lluvia o a la humedad del suelo, considera colocar la colmena en un lugar más seco o levantando la colmena del suelo sobre soportes. Además, asegúrate de que el techo de la colmena esté en buenas condiciones para evitar filtraciones de agua.");                 
            sendTelegramMessage("4) En casos extremos, puedes considerar el uso de absorbentes de humedad naturales, como bolsas de gel de sílice o cáscaras de arroz secas, dentro de la colmena para ayudar a controlar la humedad.");                 
        
        }
        
        int co2Average = parts[2].toInt();
         
        if (co2Average >= 400 && co2Average < 3000) {
            co2Total += co2Average;
            co2ReadingsInRange++;
        } else if (co2Average < 400) {
            sendTelegramMessage("¡Alerta! La concentración de CO2 está por debajo del rango seguro.⚠️");                 
       
        } else {
           sendTelegramMessage("¡Alerta! La concentración de CO2 está por encima del rango seguro.⚠️"
                                "Durante el último día de monitoreo, hemos registrado que tu colmena presenta niveles de CO2 que están por encima del rango seguro.⚠️"
                                "Como sabes, esto afecta directamente la salud de tus abejas🐝 debido a que el aire que respiran se vuelve tóxico, y puede causar problemas muy graves. Para eso, te hacemos las siguientes recomendaciones");
            sendTelegramMessage("1) Asegúrate de que la colmena tenga una ventilación adecuada para permitir que el CO2 escape y entre aire fresco. Puedes aumentar la ventilación abriendo las entradas de la colmena para mejorar el flujo de aire.");                 
            sendTelegramMessage("2) Verifica que no haya obstrucciones en las entradas de la colmena que puedan dificultar el flujo de aire, lo que podría contribuir a la acumulación de CO2 en el interior.");                 
            sendTelegramMessage("3) Una alta densidad de población de abejas puede contribuir a niveles elevados de CO2. Considera dividir la colmena si está demasiado poblada para reducir la concentración de abejas en un solo espacio.");                 
       
        }
        
        int tvocAverage = parts[3].toInt();
              
        if (tvocAverage >= 0 && tvocAverage < 2500) {
            tvocTotal += tvocAverage;
            tvocReadingsInRange++;
        } else if (tvocAverage < 0) {
            sendTelegramMessage("¡Alerta! La concentración de TVOC está por debajo del rango seguro🔥");
        } else {
            sendTelegramMessage("¡Alerta! La concentración de TVOC está por encima del rango seguro.🔥"
                                "Durante el último día de monitoreo, hemos registrado que tu colmena presenta niveles de TVOC que están por encima del rango seguro.⚠️"
                                "Como sabes, esto afecta directamente la salud de tus abejas🐝 debido a que el aire que respiran se vuelve tóxico, y puede causar problemas muy graves. Para eso, te hacemos las siguientes recomendaciones");
            sendTelegramMessage("1) Asegúrate de que la colmena tenga una ventilación adecuada para permitir que los gases TVOC escape y entre aire fresco. Puedes aumentar la ventilación abriendo las entradas de la colmena para mejorar el flujo de aire.");                 
            sendTelegramMessage("2) Intenta identificar la fuente de los compuestos orgánicos volátiles en la colmena. Puede ser causada por productos químicos agrícolas, pinturas, barnices, pesticidas o productos de limpieza utilizados en la cercanía de la colmena. Si es posible, elimina o reduce el uso de estas sustancias cerca de las abejas.");                 
            sendTelegramMessage("3) Una alta densidad de población de abejas puede contribuir a niveles elevados de TVOC. Considera dividir la colmena si está demasiado poblada para reducir la concentración de abejas en un solo espacio.");                  
        }

        readings++;

      WiFi.disconnect(); // Desconectar WiFi
      delay(22000); // Esperar 25 segundos (20000 ms)
    
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
        
        int Alerta= 0;
       
        // classify class 0
    if (!tf.predict(AlertSample).isOk()) {
        Serial.println(tf.exception.toString());
        return;
    }
    
    Serial.print("expcted class 0, predicted class ");
    Serial.println(tf.classification);
        
        Serial.print(Alerta);
            
        // Después de calcular el nivel de alerta y la suma de pesos
        
        if (Alerta == 3) {
            sendTelegramMessage("Nivel general de alerta: MUY ALTO🔴🔴?         🚨🕷️¡Alerta!🕷️🚨Parece que tus abejas están alcanzando un nivel muy alto de infestación por varroa⚠️⚠️, Superior al 10%. Por lo tanto, es necesario tomar medidas cuanto antes para evitar daños irreversibles🛡️");
            sendTelegramMessage("1) Verifica la ubicación de tu colmena📌📌, probablemente no esté localizada en un lugar adecuado. Recuerda que debe estar donde reciba la luz del sol, no tenga exceso de humedad, que esté alejada del suelo y lo principal, comprobar la presencia de otros colmenares vecinos e investigar sobre su situación");
            sendTelegramMessage("2) Realiza una limpieza general de la colmena🧴💦. la limpieza mediante el uso de soda cáustica, ácido oxálico o agua oxigenada. Al desinfectar los cuadros, se eliminan residuos de cera, propóleos, polen y posibles ácaros adheridos");
            sendTelegramMessage("3) Utiliza un tratamiento para frenar su avance, si no deseas usar productos químicos🪻,  la aplicación aceites esenciales como lavanda y laurel contribuyen al desprendimiento efectivo de los ácaros desde las abejas, provocando una caída natural de varroa. Es un tratamiento natural y que contribuye a la prevención del ácaro ");
            sendTelegramMessage("4) Tratamientos que pueden ser efectivos mediante el uso de soda cáustica, ácido oxálico o agua oxigenada💊. Al desinfectar los cuadros, se eliminan residuos de cera, propóleos, polen y posibles ácaros adheridos. ");
        
        } else if (Alerta ==0 ) {
             sendTelegramMessage("🪲");
            sendTelegramMessage("Nivel general de alerta: ALTO🟠  🐝¡Cuidado!🕷️ Parace ser que la varroa ya ha entrado a tu colmena y se encuentra en una fase de desarrollo bastante considerable⚠️, entre un 5% y un 9% 🚨. Para esto te hacemos las siguientes recomendaciones:");
            sendTelegramMessage("1) Verifica la ubicación de tu colmena📌, probablemente no esté localizada en un lugar adecuado. Recuerda que debe estar donde reciba la luz del sol, no tenga exceso de humedad, que esté alejada del suelo y lo principal, comprobar la presencia de otros colmenares vecinos e investigar sobre su situación");
            sendTelegramMessage("2) Realiza una limpieza general de la colmena🧴💦. la limpieza mediante el uso de soda cáustica, ácido oxálico o agua oxigenada. Al desinfectar los cuadros, se eliminan residuos de cera, propóleos, polen y posibles ácaros adheridos");
            sendTelegramMessage("3) Utiliza un tratamiento para frenar su avance🍃,si no deseas usar productos químicos  La aplicación aceites esenciales como lavanda y laurel contribuyen al desprendimiento efectivo de los ácaros desde las abejas, provocando una caída natural de varroa. Es un tratamiento natural y que contribuye a la prevención del ácaro ");
        
        } else if (Alerta == 2) {
            sendTelegramMessage("Nivel general de alerta: MEDIO🟡🟡             🐝¡Bien!🐝 Tu colmena parece estar sana. Sin embargo, es posible que tengas un porcentaje de infestación de varroa medio-bajo entre un 2% y un 4%. Esto significa que probablemente el ácaro esté tomando fuerza al interior de tu colmena. Para esto, te hacemos las siguientes recomendaciones:  ");
            sendTelegramMessage("1) Para tu siguiente visita al apiario, recuerda hacer una limpieza de la colmena. Elimina la cera que se encuentre en mal estado y retira el exceso de humedad");
            sendTelegramMessage("2) Recuerda que la colmena debe estar localizada en un lugar adecuado, donde reciba la luz del sol, no tenga exceso de humedad y que esté alejada del suelo");
            sendTelegramMessage("3) Si lo deseas, La aplicación aceites esenciales como lavanda y laurel contribuyen al desprendimiento efectivo de los ácaros desde las abejas, provocando una caída natural de varroa. Es un tratamiento natural y que contribuye a la prevención del ácaro ");
        
        } else if (Alerta == 1) {
            sendTelegramMessage("Nivel general de alerta: BAJO🟢🟢             🐝¡Felicitaciones!🐝 Tu colmena actualmente se encuentra bastante sana😊. Es probable que la varroa no esté presente o está en un nivel muy bajo. Tus abejas están en su máximo nivel de producción y de seguir así, lo más seguro es que tengas una buena cosecha de miel🍯. Recuerda seguir las recomendaciones para que continuen sanas🐝");
            
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
