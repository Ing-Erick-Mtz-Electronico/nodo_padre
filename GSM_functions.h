
//GSM varibles
#define TINY_GSM_MODEM_SIM800

#define HTTP_HEADER_ACCEPT_ENCODING "Accept-Encoding: "
#define HTTP_HEADER_ACCEPT "Accept: "
#define HTTP_HEADER_CONNECTION "Connection: "

String acceptEncoding = "gzip,deflate,br";
String acceptValue = "application/json";
String connection = "keep-alive";
String contentType = "text/plain";

// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

//Increase RX buffer
#define TINY_GSM_RX_BUFFER 650

#define USE_SSL
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

#define GSM_PIN "1234"

#define muestras 10

//#define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>

const char TIME_HOST[]  = "worldtimeapi.org";
const char TIME_PATH[] = "/api/timezone/America/Bogota";
const int PORT = 80;

//const char UNIMAG_HOST[] = "biblioteca3g.unimagdalena.edu.co";
//const char UNIMAG_PATH[] = "/esp32/";
//const int SSL_PORT = 80;

const char UNIMAG_HOST[] = "34.207.125.0";
const char UNIMAG_PATH[] = "/api/nodes/storage/";
const int SSL_PORT = 80;

//const char UNIMAG_HOST[] = "sistemas-inteligentes-backend.1.us-1.fl0.io";
//const char UNIMAG_PATH[] = "/api/nodes/storage/";
//const int SSL_PORT = 443;

//char apn[]  = "internet.movistar.com.co";
//char user[] = "movistar";
//char pass[] = "movistar";

char apn[]  = "internet.comcel.com.co";
char user[] = "comcel";
char pass[] = "comcel";

//GSM Module RX pin to ESP32 4
//GSM Module TX pin to ESP32 2
#define rxPin 4
#define txPin 2

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, Serial);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);
//TinyGsmClientSecure clientServer(modem,1);

boolean connectionAPN(){
  
  Serial.print(F("Connecting to "));
  Serial.println(apn);
  
  while(!modem.gprsConnect(apn,user,pass)) {
    Serial.println(" conexion fail");
    if(((millis()-lastTime)>TIME_OFF)&& flag){
      return false;
    }
    delay(1000);
  }
  Serial.println("Conectado al APN");

  while(!modem.waitForNetwork(600000L, true)){
    Serial.println("sin conexion al la red");
    if(((millis()-lastTime)>TIME_OFF)&& flag){
      return false;
    }        
  }

  if (modem.isNetworkConnected()) { 
    Serial.println("Red conectada");
    return true;
  }
  return false;
}

String getTimeHost(){

  // Make a HTTP GET request:
  Serial.println("Performing HTTP GET request...");
  client.print(String("GET ") + TIME_PATH + " HTTP/1.1\r\n");
  client.print(String("Host: ") + TIME_HOST + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.println();

  uint32_t timeout = millis();
  String response = "";
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      response += c;
      timeout = millis();
    }
  }
  
  // Separa la cabecera del cuerpo
  int headerEndIndex = response.indexOf("\r\n\r\n");
  String header = response.substring(0, headerEndIndex);
  String body = response.substring(headerEndIndex + 4);
  
  // Extrae el código de estado
  int statusCodeStartIndex = header.indexOf(" ") + 1;
  int statusCodeEndIndex = header.indexOf(" ", statusCodeStartIndex);
  int statusCode = header.substring(statusCodeStartIndex, statusCodeEndIndex).toInt();
  
  // Imprime el código de estado y el cuerpo
  Serial.print("Código de estado: ");
  Serial.println(statusCode);
  Serial.print("Cuerpo: ");
  Serial.println(body);
  Serial.println();
  client.stop();
  Serial.println(F("Server disconnected"));
  
  if(statusCode == 200){
    return body;
  }
  return "";
  
}

String requestTime(){
  
//  const char* number = "3146207276";
//  const char* sms = "pruebas desde el moden sim800";
  String request = "";
  while(request.length()==0){
    if(((millis()-lastTime)>TIME_OFF)&& flag){
      return request;
    }
    if(connectionAPN()){
      
      if(!client.connect(TIME_HOST, PORT)){
        Serial.println("HTTP  not connect");   
      }else{
        request = getTimeHost();
        return request;
      }
    }
    //modem.sendSMS(number,sms)
  }
  
}

boolean envio(String subData){
  String response;
  Serial.println("making post request");
  Serial.println(subData);

  client.print(String("POST ")+ UNIMAG_PATH + " HTTP/1.1\r\n");
  client.print(String("Host: ") + UNIMAG_HOST + "\r\n");
  client.print("Content-Type: text/plain\r\n");
  client.print("Content-Length: " + String(subData.length()) + "\r\n");
  //client.print("Accept: text/plain;charset=UTF-8\r\n");
  client.print("Accept: application/json\r\n");
  client.print("Accept-Encoding: gzip,deflate,br\r\n");
  client.print("Connection: keep-alive\r\n\r\n");
  client.print(subData+"\r\n");

  uint32_t timeout = millis();
  response = "";
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      response += c;
      //Serial.print(c);
      timeout = millis();
    }
  }
  // Separa la cabecera del cuerpo
  int headerEndIndex = response.indexOf("\r\n\r\n");
  String header = response.substring(0, headerEndIndex);
  String body = response.substring(headerEndIndex + 4);
  //int endBody = body.indexOf("}");
  //body = body.substring(0,endBody);
  
  // Extrae el código de estado
  int statusCodeStartIndex = header.indexOf(" ") + 1;
  int statusCodeEndIndex = header.indexOf(" ", statusCodeStartIndex);
  int statusCode = header.substring(statusCodeStartIndex, statusCodeEndIndex).toInt();
  Serial.print("Código de estado: ");
  Serial.println(statusCode);
  Serial.print("Cuerpo: ");
  Serial.println(body);
  client.stop();

  if(statusCode != 0){
    appendFile(SD, LOG, body.c_str());
  }

  if(statusCode == 201){
    return false;
  }
  return true;
}

boolean sendPost(){//prueba
  String response;
  //int statusCode = 0;
  String postData = leerArchivo(SD, PATH2);
  
  if(postData.length()<2){
    return true;
  }
  
  int index = 0;
  String subData = "";
  boolean flag = false;
  int countTramas = 0;
  
  for(int i=0;i<postData.length();i++){
    if(postData[i]=='\n'){
      countTramas++;
    }
  }
  
  int nEnvios = countTramas/muestras;
  if(countTramas > muestras){
    countTramas = 0;
    for(int i=0;i<postData.length();i++){
      if(flag){
        //delay(5000);
        if(connectionAPN()){
          client.connect(UNIMAG_HOST, SSL_PORT);
          for(int j=0;j<4;j++){
            flag = envio(subData);
            if(!flag){
              break;
            }
          }
        }
        if(flag){
          subData += '\n';
          subData.concat(postData.substring(i,postData.length()));
          writeFile(SD, PATH2, subData.c_str());
          return false;
        }
      }
      
      if(postData[i]=='\n'){
        countTramas++;
      }
      if(countTramas==muestras){
        subData = postData.substring(index,i);
        index = i+1;
        client.connect(UNIMAG_HOST, SSL_PORT);
        flag = envio(subData);
        countTramas = 0;
      }
    }
    if(countTramas != 0){
      subData = postData.substring(index,postData.length()-1);
      client.connect(UNIMAG_HOST, SSL_PORT);
      flag = envio(subData);
    }
  }else{
    subData = postData.substring(index,postData.length()-1);
    client.connect(UNIMAG_HOST, SSL_PORT);
    flag = envio(subData);

    if(flag){
      if(connectionAPN()){
        client.connect(UNIMAG_HOST, SSL_PORT);
        for(int j=0;j<4;j++){
          flag = envio(subData);
          if(!flag){
            break;
          }
        }
      }
      if(flag){
        return false;
      }
    }
  }
  return true;
}

boolean sendInformation(){
  
  //while(true){
    /*if(((millis()-lastTime)>TIME_OFF)&& flag){
      return false;
    }*/
    if(connectionAPN()){
      
      if(!client.connect(UNIMAG_HOST, SSL_PORT)){
        Serial.println("HTTP  not connect");  
      }else{
        return sendPost();
      }
    }else{
      return false;
    }
    
  //}
}

void setupGSM(){
  //inicializacion modulo GSM
  SerialAT.begin(9600, SERIAL_8N1, rxPin, txPin);
  Serial.println("SIM800L serial initialize");
  delay(3000);
  Serial.println("Initializing modem...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  Serial.print("status: ");
  Serial.println(modem.getSimStatus());
  if(modem.getSimStatus()!= 3 && !flag) { 
    modem.simUnlock(GSM_PIN); 
    delay(1000);
  }
}
