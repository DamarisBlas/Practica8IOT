 /*// Importamos las librerías ultimo
 //borrador
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

#include "config.h"

#define SENSOR_PIN  19 // ESP32 pin GIOP21 conectado al pin DQ del DS18B20

String HOST_NAME = "http://192.168.89.175"; // direccion IP de PC xampp


OneWire oneWire(SENSOR_PIN);  // asignando el pin de lectura
DallasTemperature DS18B20(&oneWire);

float T; // temperatura en Celsius

//reles
const int Foco = 21;
const int Vent = 18;

//SetPoint
int setPoint = 22;
int Sup;
int Inf;

String pwmValue;

AsyncWebServer server(80);

//int estadoFoco = 0;
int estadoVentilador = 0;

int controlarFoco(int estadoFoco) {  
      String PATH_NAME   = "/insert_openTrash.php";
      String queryString = "?estado="; 
            HTTPClient http;
            http.begin(HOST_NAME + PATH_NAME + queryString); //HTTP
      int httpCode = http.GET();
    
      // httpCode will be negative on error
      if(httpCode > 0) {
          // file found at server
          if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
          } else {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end(); 
    


  if (estadoFoco == 0) {
    Serial.println("Estado foco: apagado");
    digitalWrite(Foco, LOW);
    
  } else if (estadoFoco == 1) {
    Serial.println("Estado foco: encendido");
    digitalWrite(Foco, HIGH);
  }
  return estadoFoco;
}

void setup() {
  Serial.begin(115200);
  DS18B20.begin();      // initializando el sensor DS18B20
  pinMode(Vent,OUTPUT);
  pinMode(Foco,OUTPUT);

  // Iniciamos  SPIFFS
  if (!SPIFFS.begin())
  { Serial.println("ha ocurrido un error al montar SPIFFS");
    return;
  }
  // conectamos al Wi-Fi

  WiFi.begin(ssid, password);
  // Mientras no se conecte, mantenemos un bucle con reintentos sucesivos
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    // Esperamos un segundo
    Serial.println("Conectando a la red WiFi..");
  }
  Serial.println();
  Serial.println(WiFi.SSID());
  Serial.print("Direccion IP:\t");
  // Imprimimos la ip que le ha dado nuestro router
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html",String(), false);
  });

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false);
  });
  
  server.on("/sensor.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/sensor.html", String(), false);
  });

  server.on("/reloj.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/reloj.html", String(), false);
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.css", "text/css");
  });
  
  server.on("/IP", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.localIP()).c_str());
  });
  server.on("/HOSTNAME", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.getHostname()).c_str());
  });
  server.on("/STATUS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.status()).c_str());
  });
  server.on("/SSID", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.SSID()).c_str());
  });
  server.on("/PSK", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.psk()).c_str());
  });
  server.on("/BSSID", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.BSSIDstr()).c_str());
  });
  server.on("/RSSI", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.RSSI()).c_str());
  });

  //Encendido y apagado foco 
  server.on("/FocoEstado0", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("Estado foco: apagado\t");
   // digitalWrite(Foco, LOW);
   // estadoFoco = 0;
     int estadoActual = controlarFoco(0);
  String respuesta = String(estadoActual);
  request->send(200, "text/plain", respuesta);
   
  });
  server.on("/FocoEstado1", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("Estado foco: encendido\t");
    //digitalWrite(Foco, HIGH);
    //estadoFoco = 1;
int estadoActual = controlarFoco(1);
  String respuesta = String(estadoActual);
  request->send(200, "text/plain", respuesta);
  });
  //Encendido y apagado ventilador
  server.on("/VentiladorEstado0", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado ventilador: apagado\t");
    digitalWrite(Vent, LOW);
    estadoVentilador = 0;
  });
  server.on("/VentiladorEstado1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado ventilador: encendido\t");
    digitalWrite(Vent, HIGH);
    estadoVentilador = 1;
  });

  //Segunda pestaña
  server.on("/SLIDER", HTTP_POST, [](AsyncWebServerRequest *request){
    pwmValue = request->arg("pwmValue");
    Serial.print("Set_point:\t");
    Serial.println(pwmValue);
    setPoint = pwmValue.toInt();
    if (T < setPoint) {
      digitalWrite(Foco, HIGH);
      digitalWrite(Vent, LOW);
      estadoFoco = 1;
      estadoVentilador = 0;
    }
    else {
      digitalWrite(Foco, LOW);
      digitalWrite(Vent, HIGH);
      estadoFoco = 0;
      estadoVentilador = 1;
    }
    request->send(SPIFFS, "/sensor.html", String(), false);
  });

  server.on("/SensorLM35", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(T).c_str());
  });
  server.on("/GraficaFoco", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(estadoFoco).c_str());
  });
  server.on("/GraficaVentilador", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(estadoVentilador).c_str());
  });
  
  server.begin();
}

void loop() {
  DS18B20.requestTemperatures();       // enviando comando de lectura
  T = DS18B20.getTempCByIndex(0);  // lectura de temperatura en °C
  Serial.print("Temperature: ");
  Serial.println(T);                // imprimiendo temperatura en °C
  delay(500);
}*/
// Importamos las librerías
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <HTTPClient.h>
#include "esp_task_wdt.h"


#include "config.h"

#define SENSOR_PIN  19 // ESP32 pin GIOP21 conectado al pin DQ del DS18B20

OneWire oneWire(SENSOR_PIN);  // asignando el pin de lectura
DallasTemperature DS18B20(&oneWire);

String HOST_NAME = "http://192.168.89.175";
String PATH_NAME = "/HORNO.php";

String hornoId="1";
String zona="habitacion";
String estadoHorno;
String calefactor;
String enfriador;
String set_Point;
String tempProceso;
String sensorLM35;
float TemProceso; // temperatura en Celsius

//reles
const int Foco = 21;
const int Vent = 18;

//SetPoint
int setPoint = 22;
int Sup;
int Inf;

String pwmValue;

AsyncWebServer server(80);

//int estadoFoco = 1;
int calefacto = 1;
bool estadoHorn= false;
//int estadoVentilador = 1;
int enfriado = 1;


/*void SolicitudHttp(){
String queryString = "?Horno_id=" + hornoId;
queryString += "&Zona=" + zona;
queryString += "&Estado_del_Horno=" + estadoHorno;
queryString += "&Calefactor=" + calefactor;
queryString += "&Enfriador=" + enfriador;
queryString += "&Set_Point=" + set_Point;
queryString += "&TempProceso=" + tempProceso;
queryString += "&SensorLm35=" + sensorLM35;
Serial.print ("Cadena Resultante = "); Serial.println(queryString);
HTTPClient http;
http.begin(HOST_NAME + PATH_NAME + queryString);
int httpCode = http.GET();  
  if (httpCode > 0) {  
    if (httpCode == HTTP_CODE_OK) {    
      String payload = http.getString();      
      Serial.println(payload);    
    }else{
       Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString (httpCode) .c_str());
    }
  }else {
      Serial.printf("[HTTP) GET... failed, error: %s\n", http.errorToString (httpCode).c_str());
  }
http.end();
}*/
void setup() {
   esp_task_wdt_init(30, true);
  Serial.begin(115200);
  DS18B20.begin();      // initializando el sensor DS18B20
  pinMode(Vent,OUTPUT);
  pinMode(Foco,OUTPUT);

  // Iniciamos  SPIFFS
  if (!SPIFFS.begin())
  { Serial.println("ha ocurrido un error al montar SPIFFS");
    return;
  }
  // conectamos al Wi-Fi
  WiFi.begin(ssid, password);
  // Mientras no se conecte, mantenemos un bucle con reintentos sucesivos
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    // Esperamos un segundo
    Serial.println("Conectando a la red WiFi..");
  }
  Serial.println();
  Serial.println(WiFi.SSID());
  Serial.print("Direccion IP:\t");
  // Imprimimos la ip que le ha dado nuestro router
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html",String(), false);
  });

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false);
  });
  
  server.on("/sensor.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/sensor.html", String(), false);
  });

  server.on("/reloj.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/reloj.html", String(), false);
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.css", "text/css");
  });
  
  server.on("/IP", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.localIP()).c_str());
  });
  server.on("/HOSTNAME", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.getHostname()).c_str());
  });
  server.on("/STATUS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.status()).c_str());
  });
  server.on("/SSID", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.SSID()).c_str());
  });
  server.on("/PSK", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.psk()).c_str());
  });
  server.on("/BSSID", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.BSSIDstr()).c_str());
  });
  server.on("/RSSI", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(WiFi.RSSI()).c_str());
  });

  //Encendido y apagado foco
  server.on("/FocoEstado0", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado foco: apagado\t");
    digitalWrite(Foco, LOW);
    estadoHorn= false;
    estadoHorno= String(estadoHorn);
    calefacto = 0;
    calefactor = String(calefacto);
    //SolicitudHttp(estadoHorno);
   // SolicitudHttp(calefactor);
        SolicitudHttp(estadoHorno, calefactor, enfriador, set_Point);
          //SolicitudHttp(estadoHorno, calefactor);



   // Agregar un retraso de 1 segundo (1000 milisegundos)
    unsigned long delayStartTime = millis();
    while (millis() - delayStartTime < 1000) {
    // Esperar sin bloquear la ejecución
     yield(); // Permitir que otras tareas se ejecuten (si las hay)
    }

  });
  server.on("/FocoEstado1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado foco: encendido\t");
    digitalWrite(Foco, HIGH);
    estadoHorn= true;
    estadoHorno=String(estadoHorn);
    calefacto = 1;
    calefactor = String(calefacto);
   // SolicitudHttp();
   // SolicitudHttp(calefactor);
      SolicitudHttp(estadoHorno, calefactor, enfriador, set_Point);
      //SolicitudHttp(estadoHorno, calefactor);



    unsigned long delayStartTime = millis();
    while (millis() - delayStartTime < 1000) {
    // Esperar sin bloquear la ejecución
     yield(); // Permitir que otras tareas se ejecuten (si las hay)
    }

  });
  //Encendido y apagado foco
  server.on("/VentiladorEstado0", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado ventilador: apagado\t");
    digitalWrite(Vent, LOW);
    enfriado = 0;
    enfriador = String(enfriado);
    //SolicitudHttp(enfriador);
      SolicitudHttp(estadoHorno, calefactor, enfriador, set_Point);

    unsigned long delayStartTime = millis();
    while (millis() - delayStartTime < 1000) {
    // Esperar sin bloquear la ejecución
     yield(); // Permitir que otras tareas se ejecuten (si las hay)
    }

  });
  server.on("/VentiladorEstado1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Estado ventilador: encendido\t");
    digitalWrite(Vent, HIGH);
    enfriado = 1;
    enfriador = String(enfriado);
    //SolicitudHttp(enfriador);
     SolicitudHttp(estadoHorno, calefactor, enfriador, set_Point);
     //SolicitudHttp( enfriador);
    unsigned long delayStartTime = millis();
    while (millis() - delayStartTime < 1000) {
    // Esperar sin bloquear la ejecución
     yield(); // Permitir que otras tareas se ejecuten (si las hay)
    }
  });

  //Segunda pestaña
  server.on("/SLIDER", HTTP_POST, [](AsyncWebServerRequest *request){
    pwmValue = request->arg("pwmValue");
    Serial.print("Set_point:\t");
    Serial.println(pwmValue);
    setPoint = pwmValue.toInt();
    set_Point = String(setPoint);
    if (TemProceso < setPoint) {
      digitalWrite(Foco, HIGH);
      digitalWrite(Vent, LOW);
      calefacto = 1;
      calefactor = String(calefacto);
      enfriado = 0;
      enfriador = String(enfriado);
      
      unsigned long delayStartTime = millis();
    while (millis() - delayStartTime < 1000) {
    // Esperar sin bloquear la ejecución
     yield(); // Permitir que otras tareas se ejecuten (si las hay)
    }
      
    }
    else {
      digitalWrite(Foco, LOW);
      digitalWrite(Vent, HIGH);
      calefacto = 0;
      calefactor = String(calefacto);
      enfriado = 1;
      enfriador = String(enfriado);     
      unsigned long delayStartTime = millis();
    while (millis() - delayStartTime < 1000) {
    // Esperar sin bloquear la ejecución
     yield(); // Permitir que otras tareas se ejecuten (si las hay)
    }
    }
   //SolicitudHttp(calefactor);
   //SolicitudHttp(enfriador);
   // SolicitudHttp(set_point);
    SolicitudHttp(estadoHorno, calefactor, enfriador, set_Point);
     //SolicitudHttp( calefactor, enfriador, set_Point);




    request->send(SPIFFS, "/sensor.html", String(), false);
  });

  server.on("/SensorLM35", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(TemProceso).c_str());
  });
  server.on("/GraficaFoco", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(calefacto).c_str());
  });
  server.on("/GraficaVentilador", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(enfriado).c_str());
  });
  
  server.begin();
}
  
void loop() {
  DS18B20.requestTemperatures();       // enviando comando de lectura
  TemProceso = DS18B20.getTempCByIndex(0);
  //TempProceso = TemProceso;
  sensorLM35=String(TemProceso);
  // lectura de temperatura en °C
  Serial.print("Temperature: ");
  Serial.println(TemProceso);                // imprimiendo temperatura en °C
  delay(500);
  //SolicitudHttp();
}
  void SolicitudHttp(String estadoHorno, String calefactor, String enfriador, String set_Point){
String queryString = "?Horno_id=" + hornoId;
queryString += "&Zona=" + zona;
queryString += "&Estado_del_Horno=" + estadoHorno;
queryString += "&Calefactor=" + calefactor;
queryString += "&Enfriador=" + enfriador;
queryString += "&Set_Point=" + set_Point;
queryString += "&TempProceso=" + tempProceso;
queryString += "&SensorLm35=" + sensorLM35;

Serial.print ("Cadena Resultante = "); 
Serial.println(queryString);
HTTPClient http;
http.begin(HOST_NAME + PATH_NAME + queryString);
int httpCode = http.GET();  

  if (httpCode > 0) {  
    if (httpCode == HTTP_CODE_OK) {    
      String payload = http.getString();      
      Serial.println(payload);    
    }else{
       Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString (httpCode) .c_str());
    }
  }else {
      Serial.printf("[HTTP) GET... failed, error: %s\n", http.errorToString (httpCode).c_str());
  }
http.end();
}
