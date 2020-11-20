#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "index.h"
const char *ssid = "HONDA_JAZZ";
const char *password = "password";

bool ledState = 1;
const int ledPin = 2;


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients() {
  String str = "{\"temp\":\"30\",\"status\":" + String(ledState) +"}";
  ws.textAll(str);
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
      return "OFF";
    }
    else{
      return "ON";
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  WiFi.mode(WIFI_AP); // ใช้งาน WiFi ในโหมด AP
  WiFi.softAP(ssid, password);  //or WiFi.softAP(ssid, password)
  Serial.print("Connect to : "+ String(ssid));
  IPAddress apip = WiFi.softAPIP();
  Serial.print("visit: " + apip);
  
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
