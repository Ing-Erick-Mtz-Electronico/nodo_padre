//variable que indica el inicio de la esp
#define ID_NODO ("1")
//#define pinSensors 27
//#define pinGSM 32

#include <ESP32Time.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>

unsigned long lastTime = millis();


//Red tigo
//char apn[]  = "web.colombiamovil.com.co";
//char user[] = NULL;
//char pass[] = NULL;

#define SAMPLING_WINDOW 3600000 //240000 4 minutos hora //3600000 1 hora 
#define RECEPTION_WINDOW 180000 //600000 //10 minutos ventana de recepción //600000 10 minutos
#define TIME_OFF 180000//600000 // se levanta 60000 10 minutos antes de que los nodos se conecten //600000 10 minutos
#define mS_TO_uS_FACTOR 1000 //factor para pasar milis a micro segundos
RTC_DATA_ATTR boolean flag = false; //se indica si el reset ha sido por el mmodo deep sleep
RTC_DATA_ATTR boolean rebootNow = false; //se indica si el reset ha sido por el mmodo deep sleep
//usb
#include "usb_functions.H";
//GSM
#include "GSM_functions.H";

//funciones para las mediciones
#include "measure_functions.H";

//reloj interno de la esp
ESP32Time rtc;

//variables del deep sleep
void deepSleep();

//Servidor
// Creamos nuestra propia red -> SSID & Password
const char* ssid = "GIDEAMSERVER";  
const char* password = "1234567890";
AsyncWebServer server(80);

unsigned long initTimeService = 0;//marca el tiempo de inicio de los servicios

void setup() {
  
  Serial.begin(115200);

  while(!Serial) {
    Serial.print("."); // Espera hasta que el puerto serial se conecte
  }

//  pinMode(pinSensors,OUTPUT);
//  pinMode(pinGSM,OUTPUT);
//
//  digitalWrite(pinSensors,HIGH);
//  digitalWrite(pinGSM,HIGH);
  
  setupUsb();
  setupMeasure();
  setupGSM();

  const char* number = "3146940325";
  const char* numberMiguel = "3003859853";
  const String sms = AlertBatery();

  if(sms.length()>1){
    modem.sendSMS(number,sms.c_str());
    delay(3000);
    modem.sendSMS(numberMiguel,sms.c_str());
  }
  
  
  //configurar la hora
  String responseTime = requestTime();
  if(responseTime.length()!=0){
    JSONVar objectRequest = JSON.parse(responseTime);
    JSONVar timeRequest = objectRequest["unixtime"];
    JSONVar offsetRequest = objectRequest["raw_offset"];
    rtc.offset = long(offsetRequest);
    rtc.setTime(long(timeRequest));
  }
  
  //TOMAR MEDICIONES
  String dataMeasurement = measurement(rtc.getTime("%FT%T"));
  appendFile(SD, PATH, dataMeasurement.c_str());
  appendFile(SD, PATH2, dataMeasurement.c_str());

  if(rebootNow){
    while(!((millis()-lastTime)>TIME_OFF));
  }
  
  // Creamos el punto de acceso
  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  IPAddress getway = ip;
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(ip,getway,subnet);
  
  Serial.print("IP esp32: ");
  Serial.println(ip);
  Serial.print("Nombre de red esp32: ");
  Serial.println(ssid);

  if(!WiFi.config(ip,getway,subnet)){
   Serial.println("error DHCP"); 
  } else {
    Serial.println("Conectado server DHCP");
  }

  //servicios
  #include "servicios.H";
  
  server.begin();
  Serial.println("Servidor HTTP iniciado");
  //deep sleep
  deepSleep();
}

void loop() {}

void deepSleep(){

  while(true){
    if((millis()-initTimeService) > RECEPTION_WINDOW){
      //APAGAR WIFI
      WiFi.mode(WIFI_OFF);
      
      //ENVIAR INFORMACION
      if(sendInformation()){
        rewriteFile(SD, PATH2);
      }
      //modo deep sleep
      unsigned long sleepTime = SAMPLING_WINDOW - (millis() - lastTime);
      Serial.println(sleepTime);
      if(sleepTime <0){
        rebootNow = false;
        Serial.println("entrando a modo deep sleep");
//        digitalWrite(pinSensors,LOW);
//        digitalWrite(pinGSM,LOW);
        esp_sleep_enable_timer_wakeup(3000 * mS_TO_uS_FACTOR);
        esp_deep_sleep_start();
      }
      Serial.println("entrando a modo deep sleep");
//      digitalWrite(pinSensors,LOW);
//      digitalWrite(pinGSM,LOW);
      rebootNow = true;
      esp_sleep_enable_timer_wakeup(sleepTime * mS_TO_uS_FACTOR);
      Serial.flush(); 
      esp_deep_sleep_start();
    }
  }
}
