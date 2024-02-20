#include <Wire.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_ADDRESS (0x76)

//variables HD38
#define sensorPin 26 // Pin analógico conectado al sensor

//divisor de tension
#define bateryPin 33

//varibles SOIL NPK MODBUS
#define RE 15
#define RXD2 16
#define TXD2 17

Adafruit_BME280 bme;

String readBME280Temperature() {
  while(!bme.begin(BME_ADDRESS));
  // Read temperature as Celsius (the default)
  float t = bme.readTemperature();
  // Convert temperature to Fahrenheit
  //t = 1.8 * t + 32;
  if (isnan(t)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(String("temperatura BME: ")+String(t));
    return String(t);
  }
}

String readBME280Humidity() {
  while(!bme.begin(BME_ADDRESS));
  float h = bme.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "0";
  }
  else {
    Serial.println(String("humedad BME: ")+String(h));
    return String(h);
  }
}

String readBME280Pressure() {
  while(!bme.begin(BME_ADDRESS));
  float p = bme.readPressure() / 100.0F;
  if (isnan(p)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "0";
  }
  else {
    Serial.println(String("presion BME: ")+String(p));
    return String(p);
  }
}
String readBME280Altitude() {
  while(!bme.begin(BME_ADDRESS));
  float a = bme.readAltitude(SEALEVELPRESSURE_HPA);
  
  if (isnan(a)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "0";
  }
  else {
    Serial.println(String("altitud BME: ")+String(a));
    return String(a);
  }
}

String readDH38Humidity(){
  int sensorValue = 0; // Variable para almacenar el valor leído
  int humedad = 0;
  sensorValue = analogRead(sensorPin);
  humedad = map(sensorValue, 0, 4095, 100, 0);
  return String(humedad);
}

String readBatery(){
  int sensorValue = 0;
  float voltaje = 0.0;
  sensorValue = analogRead(bateryPin);
  voltaje = (sensorValue*(13.0/4096.0));
  return String(voltaje);
}

String AlertBatery(){
  int sensorValue = 0;
  float voltaje = 0.0;
  sensorValue = analogRead(bateryPin);
  voltaje = (sensorValue*(13.0/4096.0));

  if(voltaje <= 10.0){
    String resl = "";
    resl.concat("ALERTA!!\n\nNivel de bateria nodo padre: ");
    resl+= String(voltaje);
    return  resl;
  }else{
    return "";  
  }
}

String soilData(){
  // Modbus request for reading all values
  const byte allMeasure[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};

  // Recoleccion de valores
  byte values[19];
  
  uint16_t humidityInt,temperatureInt,conductivity,PHInt,nitro,phos,pota = 0;
  float humidity,temperature,PH = 0.0;
  
  digitalWrite(RE,HIGH);
  
  if(Serial2.write(allMeasure,sizeof(allMeasure))==8){
    delay(18);
    digitalWrite(RE,LOW);
    delay(12);

    Serial2.readBytes(values,sizeof(values));
    
    humidityInt = (values[3] << 8) | values[4];
    humidity = humidityInt/10.0F;
    
    temperatureInt = (values[5] << 8) | values[6];
    temperature = temperatureInt/10.0F;

    conductivity = (values[7] << 8) | values[8];
    
    PHInt = (values[9] << 8) | values[10];
    PH = PHInt/10.0F;
    
    nitro = (values[11] << 8) | values[12];
    
    phos = (values[13] << 8) | values[14];
    
    pota = (values[15] << 8) | values[16];
    
    /*Serial.print("humedad: ");
    Serial.print(humidity);
    Serial.println();

    Serial.print("temperatura: ");
    Serial.print(temperature);
    Serial.println();
    
    Serial.print("conductividad: ");
    Serial.print(conductivity);
    Serial.println();
    
    Serial.print("PH: ");
    Serial.print(PH);
    Serial.println();

    Serial.print("Nitrogeno: ");
    Serial.print(nitro);
    Serial.println();

    Serial.print("Fosforo: ");
    Serial.print(phos);
    Serial.println();

    Serial.print("potasio: ");
    Serial.print(pota);
    Serial.println();*/

  }
  String result = "";
  result += String(humidity);
  result.concat(separador);
  result += String(temperature);
  result.concat(separador);
  result += String(conductivity);
  result.concat(separador);
  result += String(PH);
  result.concat(separador);
  result += String(nitro);
  result.concat(separador);
  result += String(phos);
  result.concat(separador);
  result += String(pota);
  
  return result;
}

String measurement(String timeNow){
  char finalizador = '\n';
  
  Serial.println("Medicion inicida");
  
  String object_med = ID_NODO;
  object_med.concat(separador);
  object_med.concat(timeNow);
  object_med.concat(separador);
  delay(500);
  object_med.concat(readBME280Temperature());
  object_med.concat(separador);
  delay(500);
  object_med.concat(readBME280Humidity());
  object_med.concat(separador);
  delay(500);
  object_med.concat(readBME280Pressure());
  object_med.concat(separador);
  delay(500);
  object_med.concat(readBME280Altitude());
  object_med.concat(separador);
  object_med.concat(readDH38Humidity());
  object_med.concat(separador);
  object_med.concat(soilData());
  object_med.concat(separador);
  object_med.concat(readBatery());
  object_med.concat(finalizador);
  Serial.println(object_med);
  
  return object_med;
}

void setupMeasure(){
  //Serial para el sensor SOIL
  Serial2.begin(4800, SERIAL_8N1, RXD2, TXD2);
  pinMode(RE, OUTPUT);
  //digitalWrite(RE,HIGH);

  Wire.begin();  //enable I2C port.
  if(!bme.begin(BME_ADDRESS)){
    Serial.println("No hay un módulo BME conectado");
    //delay(2000);
  }else{
    Serial.println("BME conectado");
  }
}
