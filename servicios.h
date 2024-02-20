initTimeService = millis();

server.on("/hora", HTTP_GET, [](AsyncWebServerRequest *request){
  char buf[20];
  sprintf(buf,"%lu",rtc.getEpoch());
  request->send_P(200, "text/plain", buf);
});

server.on("/data", HTTP_POST, [](AsyncWebServerRequest *request){
  unsigned long Delay = millis() - initTimeService;
  request->send(200, "text/plain", String(Delay));
  },NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  String body = "";
  
  for(size_t i=0; i<len; i++){
    body += (char)data[i];
  }
  appendFile(SD, PATH, body.c_str());
  appendFile(SD, PATH2, body.c_str());
  Serial.println(body);
 });
