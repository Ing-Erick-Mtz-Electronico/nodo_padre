unsigned long lastTime = millis();

// variable que indica el inicio de la esp
#define ID_NODO ("1")
#define PATH ("/CSVrecoleccionNodoPadre1.txt")
#define PATH2 ("/CopiaCSVrecoleccionNodoPadre1.txt")
// #define pinSensors 27
// #define pinGSM 32

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>

#define SAMPLING_WINDOW 3600000    // 900000//3600000 //240000 4 minutos hora //3600000 1 hora
#define RECEPTION_WINDOW 420000    // 7minutos //180000 //3 minutos //600000 //10 minutos ventana de recepción //600000 10 minutos
#define TIME_OFF 180000            // se levanta 3 minutos antes de que los nodos se conecten //600000 10 minutos
#define TIME_LIMIT_CONECTION 60000 // tiempo de espera cuando no se conecta a internet //600000 10 minutos
#define CONECTION_SD_TIME 12000    // tiempo que chequea si la SD funciona
#define TIME_CHECK_SESNOR 12000    // tiempo que chequea si los sensores funcionan
#define mS_TO_uS_FACTOR 1000       // factor para pasar milis a micro segundos
#define S_TO_mS_FACTOR 60000       // factor para pasar segundos a milisegundos segundos
#define NODO_HIJO_TIME_DELAY 5000  // tiempo para que el nodo hijo se sincronice
#define delayLed 1000

#define LED 32
boolean stateLed = true;

#define PIN_CONTROL 25

void serverLoop();
void sleepEsp(unsigned long sleepTime);

// usb
#include "usb_functions.H";
// GSM
#include "GSM_functions.H";

// funciones para las mediciones
#include "measure_functions.H";

#include "tinyRTC_functions.H";

//  Creamos nuestra propia red -> SSID & Password
const char *ssid = "GIDEAMSERVER";
const char *password = "1234567890";
AsyncWebServer server(80);

unsigned long initTimeService = 0; // marca el tiempo de inicio de los servicios

void setup()
{

  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, stateLed);

  pinMode(PIN_CONTROL,OUTPUT);
  digitalWrite(PIN_CONTROL,stateLed);

  while (!Serial)
  {
    Serial.print("."); // Espera hasta que el puerto serial se conecte
  }

  Serial.print("Nodo: ");
  Serial.println(ID_NODO);

  //  pinMode(pinSensors,OUTPUT);
  //  pinMode(pinGSM,OUTPUT);
  //
  //  digitalWrite(pinSensors,HIGH);
  //  digitalWrite(pinGSM,HIGH);

  setupUsb();
  setupMeasure();
  setupGSM();
  setupRTC();

  const String sms = AlertBatery();

  if (sms.length() > 1)
  {
    modem.sendSMS(numberErick, sms.c_str());
    delay(1000);
    modem.sendSMS(numberMiguel, sms.c_str());
    delay(1000);
    modem.sendSMS(numberYesica, sms.c_str());
  }

  // configurar la hora
  String responseTime = requestTime();
  if (responseTime.length() != 0)
  {
    JSONVar objectRequest = JSON.parse(responseTime);
    setTimeRTC(objectRequest);
    responseTime = "";
  }

  // TOMAR MEDICIONES
  String dataMeasurement = measurement(getTimeRTC());
  appendFile(SD, PATH, dataMeasurement.c_str());
  appendFile(SD, PATH2, dataMeasurement.c_str());

  // Creamos el punto de acceso
  WiFi.softAP(ssid, password, 1, 0, 7);
  IPAddress ip = WiFi.softAPIP();
  IPAddress getway = ip;
  IPAddress subnet(255, 255, 255, 0);

  WiFi.softAPConfig(ip, getway, subnet);
  Serial.print("IP esp32: ");
  Serial.println(ip);
  Serial.print("Nombre de red esp32: ");
  Serial.println(ssid);

  if (!WiFi.config(ip, getway, subnet))
  {
    Serial.println("error DHCP");
  }
  else
  {
    Serial.println("Conectado server DHCP");
  }

  // servicios
  #include "servicios.H";

  server.begin();
  Serial.println("Servidor HTTP iniciado");

  // loop
  serverLoop();
}

void loop() {}

void serverLoop()
{
  long timeSpan = TIME_OFF - millis() - lastTime;
  long lastTimeLed = millis();
  while (true)
  {
    if ((millis() - initTimeService) > (RECEPTION_WINDOW + timeSpan))
    {
      digitalWrite(LED,true);

      //terminar servidor
      server.end();

      // APAGAR WIFI
      WiFi.mode(WIFI_OFF);

      // ENVIAR INFORMACION
      if (sendInformation())
      {
        rewriteFile(SD, PATH2);
      }
      // apagar modulo GSM
      modem.poweroff();

      // modo deep sleep
      unsigned long minuteToMicroSec = getMinute()*S_TO_mS_FACTOR;
      unsigned long sleepTime = SAMPLING_WINDOW - TIME_OFF - minuteToMicroSec;
      sleepEsp(sleepTime);
    }

    if ((millis() - lastTimeLed) > delayLed)
    {
      stateLed = !stateLed;
      digitalWrite(LED, stateLed);
      lastTimeLed = millis();
    }
  }
}

void sleepEsp(unsigned long sleepTime){
  Serial.println("entrando a modo deep sleep");
  digitalWrite(LED,false);
  digitalWrite(PIN_CONTROL,false);
  Serial.println(sleepTime);
  // sleepTime = 10000;
  // Serial.println(sleepTime);
  esp_sleep_enable_timer_wakeup(sleepTime * mS_TO_uS_FACTOR);
  esp_deep_sleep_start();
}
