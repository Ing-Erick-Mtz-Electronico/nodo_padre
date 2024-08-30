
// GSM varibles
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

// Increase RX buffer
#define TINY_GSM_RX_BUFFER 650

#define USE_SSL
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

#define GSM_PIN "1234"

#define muestras 10

// #define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>

const char TIME_HOST[] = "worldtimeapi.org";
const char TIME_PATH[] = "/api/timezone/America/Bogota";
const int PORT = 80;

// const char UNIMAG_HOST[] = "biblioteca3g.unimagdalena.edu.co";
// const char UNIMAG_PATH[] = "/esp32/";
// const int SSL_PORT = 80;

// const char UNIMAG_HOST[] = "34.207.125.0";
const char UNIMAG_HOST[] = "sistemasinteligentes.unimagdalena.edu.co";
const char UNIMAG_PATH[] = "/api/nodes/storage/";
const int SSL_PORT = 80;

// const char UNIMAG_HOST[] = "sistemas-inteligentes-backend.1.us-1.fl0.io";
// const char UNIMAG_PATH[] = "/api/nodes/storage/";
// const int SSL_PORT = 443;

// //Red movistar
// char apn[]  = "internet.movistar.com.co";
// char user[] = "movistar";
// char pass[] = "movistar";

// //Red claro
// char apn[]  = "internet.comcel.com.co";
// char user[] = "comcel";
// char pass[] = "comcel";

// Red tigo
// char apn[]  = "web.colombiamovil.com.co";
// char user[] = NULL;
// char pass[] = NULL;

String apn;
String user;
String pass;

const char *numberErick = "3146940325";
const char *numberMiguel = "3003859853";
const char *numberYesica = "3188015572";

// GSM Module RX pin to ESP32 4
// GSM Module TX pin to ESP32 2
#define rxPin 4
#define txPin 2

TinyGsm modem(SerialAT);

TinyGsmClient client(modem);
// TinyGsmClientSecure clientServer(modem,1);

boolean connectionAPN()
{

  unsigned long lastTimeApn = millis();
  Serial.print(F("Connecting to "));
  Serial.println(apn);

  while (!modem.gprsConnect(apn.c_str(), user.c_str(), pass.c_str()))
  {
    if (((millis() - lastTimeApn) > TIME_LIMIT_CONECTION))
    {
      return false;
    }
    Serial.println(" conexion fail");
    delay(1000);
  }
  Serial.println("Conectado al APN");

  while (!modem.waitForNetwork(10000, true))
  {
    Serial.println("sin conexion al la red");
    if (((millis() - lastTimeApn) > TIME_LIMIT_CONECTION))
    {
      return false;
    }
  }

  if (modem.isNetworkConnected())
  {
    Serial.println("Red conectada");
    return true;
  }
  return false;
}

String getTimeHost()
{

  // Make a HTTP GET request:
  Serial.println();
  Serial.println("HTTP GET request time...");
  client.print(String("GET ") + TIME_PATH + " HTTP/1.1\r\n");
  client.print(String("Host: ") + TIME_HOST + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.println();

  uint32_t timeout = millis();
  String response = "";
  while (client.connected() && millis() - timeout < 10000L)
  {
    // Print available data
    while (client.available())
    {
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
  // Serial.print("Cuerpo: ");
  // Serial.println(body);
  Serial.println();
  client.stop();

  if (statusCode == 200)
  {
    return body;
  }
  return "";
}

String requestTime()
{
  unsigned long lastTimeRequest = millis();
  String request = "";
  while (request.length() == 0)
  {
    if (((millis() - lastTimeRequest) > TIME_LIMIT_CONECTION))
    {
      return request;
    }
    if (connectionAPN())
    {

      if (!client.connect(TIME_HOST, PORT))
      {
        Serial.println("HTTP  not connect");
      }
      else
      {
        request = getTimeHost();
        return request;
      }
    }
  }
  return request;
}

boolean envio(String subData)
{
  stateLed = !stateLed;
  digitalWrite(LED,stateLed);
  String response;
  Serial.println("making post request");
  Serial.println(subData);

  client.print(String("POST ") + UNIMAG_PATH + " HTTP/1.1\r\n");
  client.print(String("Host: ") + UNIMAG_HOST + "\r\n");
  client.print("Content-Type: text/plain\r\n");
  client.print("Content-Length: " + String(subData.length()) + "\r\n");
  // client.print("Accept: text/plain;charset=UTF-8\r\n");
  client.print("Accept: application/json\r\n");
  client.print("Accept-Encoding: gzip,deflate,br\r\n");
  client.print("Connection: keep-alive\r\n\r\n");
  client.print(subData + "\r\n");

  uint32_t timeout = millis();
  response = "";
  while (client.connected() && millis() - timeout < 10000L)
  {
    // Print available data
    while (client.available())
    {
      char c = client.read();
      response += c;
      // Serial.print(c);
      timeout = millis();
    }
  }
  // Separa la cabecera del cuerpo
  int headerEndIndex = response.indexOf("\r\n\r\n");
  String header = response.substring(0, headerEndIndex);
  String body = response.substring(headerEndIndex + 4);
  // int endBody = body.indexOf("}");
  // body = body.substring(0,endBody);

  // Extrae el código de estado
  int statusCodeStartIndex = header.indexOf(" ") + 1;
  int statusCodeEndIndex = header.indexOf(" ", statusCodeStartIndex);
  int statusCode = header.substring(statusCodeStartIndex, statusCodeEndIndex).toInt();
  Serial.print("Código de estado: ");
  Serial.println(statusCode);
  Serial.print("Cuerpo: ");
  Serial.println(body);
  client.stop();

  if (statusCode != 0)
  {
    appendFile(SD, LOG, body.c_str());
  }

  if (statusCode == 201)
  {
    return false;
  }
  return true;
}

boolean sendPost()
{ // prueba
  String response;
  // int statusCode = 0;
  String postData = leerArchivo(SD, PATH2);

  if (postData.length() < 2)
  {
    return true;
  }

  int index = 0;
  String subData = "";
  boolean flag = false;
  int countTramas = 0;

  for (int i = 0; i < postData.length(); i++)
  {
    if (postData[i] == '\n')
    {
      countTramas++;
    }
  }

  int nEnvios = countTramas / muestras;
  if (countTramas > muestras)
  {
    countTramas = 0;
    for (int i = 0; i < postData.length(); i++)
    {
      if (flag)
      {
        // delay(5000);
        if (connectionAPN())
        {
          client.connect(UNIMAG_HOST, SSL_PORT);
          for (int j = 0; j < 4; j++)
          {
            flag = envio(subData);
            if (!flag)
            {
              break;
            }
          }
        }
        if (flag)
        {
          subData += '\n';
          subData.concat(postData.substring(i, postData.length()));
          writeFile(SD, PATH2, subData.c_str());
          return false;
        }
      }

      if (postData[i] == '\n')
      {
        countTramas++;
      }
      if (countTramas == muestras)
      {
        subData = postData.substring(index, i);
        index = i + 1;
        client.connect(UNIMAG_HOST, SSL_PORT);
        flag = envio(subData);
        countTramas = 0;
      }
    }
    if (countTramas != 0)
    {
      subData = postData.substring(index, postData.length() - 1);
      client.connect(UNIMAG_HOST, SSL_PORT);
      flag = envio(subData);
    }
  }
  else
  {
    subData = postData.substring(index, postData.length() - 1);
    client.connect(UNIMAG_HOST, SSL_PORT);
    flag = envio(subData);

    if (flag)
    {
      if (connectionAPN())
      {
        client.connect(UNIMAG_HOST, SSL_PORT);
        for (int j = 0; j < 4; j++)
        {
          flag = envio(subData);
          if (!flag)
          {
            break;
          }
        }
      }
      if (flag)
      {
        return false;
      }
    }
  }
  return true;
}

boolean sendInformation()
{

  // while(true){
  /*if(((millis()-lastTime)>TIME_OFF)&& flag){
    return false;
  }*/
  if (connectionAPN())
  {

    if (!client.connect(UNIMAG_HOST, SSL_PORT))
    {
      Serial.println("HTTP  not connect");
      return false;
    }
    else
    {
      return sendPost();
    }
  }
  else
  {
    return false;
  }

  //}
}

void setupGSM()
{
  // inicializacion modulo GSM
  SerialAT.begin(9600, SERIAL_8N1, rxPin, txPin);
  Serial.println("SIM800L serial initialize");
  delay(3000);
  Serial.println("Initializing modem...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  modemInfo = modem.getIMSI();
  if (modemInfo.length() > 0)
  {
    String opr = modemInfo.substring(3, 6);

    if (opr.toInt() == 123) // red de movistar
    {
      apn = "internet.movistar.com.co";
      user = "movistar";
      pass = "movistar";
    }
    else if (opr.toInt() == 101) // red de claro - comcel
    {
      apn = "internet.comcel.com.co";
      user = "comcel";
      pass = "comcel";
    }
    else if (opr.toInt() == 111) // red de TIGO
    {
      apn = "web.colombiamovil.com.co";
      user = "";
      pass = "";
    }

    Serial.print("status: ");
    Serial.println(modem.getSimStatus());
    if (modem.getSimStatus() != 3)
    {
      modem.simUnlock(GSM_PIN);
      delay(1000);
    }
  }
}
