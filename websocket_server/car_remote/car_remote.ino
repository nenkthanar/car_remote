#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "html.h"

#define AppName "HONDAJAZZ"
const char* ssid     = AppName;
const char* password = "12345678";
bool ledState = 0;
const int ledPin = 2;

  #ifdef __cplusplus
    extern "C" {
  #endif
    uint8_t temprature_sens_read();
  #ifdef __cplusplus
     }
  #endif
uint8_t temprature_sens_read();

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients() {
  String str = "{\"name\": \"temp\",\"val\":" +  CpuTemp() + ",\"status\":" +  ledState +"}";
  ws.textAll(str);
  Serial.println(ledState);
  }

String CpuTemp(){
  int temp = ((temprature_sens_read() - 32) / 1.8);
  return String(temp);
  }

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      ledState = !ledState;
      notifyClients();
    }
   }
   }

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    }
    }

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
   return "LOCK";
    }
    else{
   return "UNLOCK";
  }
  }
  }

void StartAP(){// start access point mode
  Serial.printf("\nConfiguring access point...\n");
  WiFi.softAP(ssid, password);
  WiFi.softAPsetHostname(ssid);
  Serial.printf("AP IP address: %s\n",WiFi.softAPIP().toString().c_str());
  }
  
void setup(){
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  StartAP();
  initWebSocket();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.begin();
  }

void loop() {
  ws.cleanupClients();
  digitalWrite(ledPin, ledState);
  }
