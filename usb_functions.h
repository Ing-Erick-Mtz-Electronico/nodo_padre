//lib SPI para leer y escribir en la micro SD
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#define PATH ("/CSVrecoleccionNodoPadre1.txt")
#define PATH2 ("/CopiaCSVrecoleccionNodoPadre1.txt")
#define LOG ("/log.txt")
#define CONSOLE_LOG ("/console_log.txt")

char separador = ';';

void consoleLog(fs::FS &fs,const char * message){
  const char * path = CONSOLE_LOG;
  File file = fs.open(path, FILE_APPEND);
  
  if(!file){
    Serial.println("Error abriendo archivo CONSOLE_LOG");
    return;
  }
  if(file.print(message)){
      Serial.println("CONSOLE_LOG appended");
  } else {
    Serial.println("CONSOLE_LOG failed");
  }
  file.close();
}

String leerArchivo(fs::FS &fs, const char * path){
  
  String payload ="";
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(file){
    while(file.available()){
      char ch = file.read();
      payload += ch;
    }
    file.close();
    Serial.println(" Archivo leido");
    //Serial.println(payload);
    return  payload;
  } else {
    Serial.println("Error leyendo el archivo");
    return "";
  } 
}

//check para archivo csv
boolean checkChar(fs::FS &fs, const char * path, String headerCheck){
  File file = fs.open(path);
  String header="";
  if(!file){
    Serial.println("Failed to open file for reading");
    return false;
  }

  Serial.print("Read from file for check char: ");
  while(file.available()){
    char charFile = file.read();
    header += charFile;
    if(charFile == '\n'){
      file.close();
      /*for(i=0;i<headerCheck.length();){
        
      }*/
      if(header == headerCheck){
        return true;
      }
      return false;
    }
  }
  file.close();
  return false;
}

boolean checkFile(fs::FS &fs, const char * path){
  
  File file = fs.open(path);
  if(!file){
    Serial.println("El archivo buscado no existe");
    file.close();
    return false;
  }
  
  Serial.printf("verificando archivo: %s\n", path);
  if(file.available()){
    file.close();
    return true;
  }
   
}

void writeFile(fs::FS &fs, const char * path,const String mensaje) {
  Serial.printf("Escribiendo el archivo: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Error al abrir el archivo para escribirlo");
    return;
  }
  if (file.print(mensaje)) {
    Serial.println("Archivo escrito");
  } else {
    Serial.println("Error al escribir");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void rewriteFile(fs::FS &fs, const char * path){
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Error al abrir el archivo para Borrarlo");
    return;
  }
  file.seek(0);
  file.print("");
  file.close();
}

void setupUsb(){
  
  String init = "ID_NODO";
         init.concat(separador);
         init.concat("fecha");
         init.concat(separador);
         init.concat("temperatura_BME");
         init.concat(separador);
         init.concat("humedad_BME");
         init.concat(separador);
         init.concat("presion_BME");
         init.concat(separador);
         init.concat("altitud_BME");
         init.concat(separador);
         init.concat("humedad_HD38");
         init.concat(separador);
         init.concat("humedad_SOIL");
         init.concat(separador);
         init.concat("temperatura_SOIL");
         init.concat(separador);
         init.concat("conductividad_SOIL");
         init.concat(separador);
         init.concat("PH_SOIL");
         init.concat(separador);
         init.concat("nitrogeno_SOIL");
         init.concat(separador);
         init.concat("fosforo_SOIL");
         init.concat(separador);
         init.concat("potasio_SOIL");
         init.concat(separador);
         init.concat("nivel_bateria\n");
         
  Serial.println("Inicializando SD card...");
  while(!SD.begin(5)) {
    Serial.println("Inicialización fallida!");
    delay(2000);
  }
  Serial.println("Inicialización lista.");

//  if(!checkFile(SD, CONSOLE_LOG)){
//    writeFile(SD, CONSOLE_LOG, String("").c_str());
//  }
  
  if(checkFile(SD, PATH)){
    if(!checkChar(SD, PATH, init)){
      writeFile(SD, PATH, init.c_str());
    }
  }else{
    writeFile(SD, PATH, init.c_str());
  }
  Serial.println("Archivo verificado");
}
