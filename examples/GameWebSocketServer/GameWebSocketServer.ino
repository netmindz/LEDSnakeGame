#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <SoftwareSerial.h>
#include <ESP8266mDNS.h>

#include <LEDSnakeGame.h>

// Must copy and paste from main game
#define MAX_SNAKES 4

#include "wifi.h"
// Create file with the following
// *************************************************************************
// #define SECRET_SSID "";
// #define SECRET_PSK "";
// *************************************************************************
const char ssid[] = SECRET_SSID;
const char passphrase[] = SECRET_PSK;

WebSocketsServer webSocket = WebSocketsServer(81);

#define SERIAL_DEBUG Serial

SoftwareSerial swSer(D3, D4); //, false);


void setup() {
  SERIAL_DEBUG.begin(115200);
  swSer.begin(115200);

  //Serial.setDebugOutput(true);
  SERIAL_DEBUG.setDebugOutput(true);

  SERIAL_DEBUG.println();
  SERIAL_DEBUG.println();
  SERIAL_DEBUG.println();

  for (uint8_t t = 4; t > 0; t--) {
    SERIAL_DEBUG.printf("[SETUP] BOOT WAIT %d...\n", t);
    SERIAL_DEBUG.flush();
    delay(1000);
  }

  // Make sure you're in station mode
  WiFi.mode(WIFI_STA);

  Serial.println("");
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  if (passphrase != NULL)
    WiFi.begin(ssid, passphrase);
  else
    WiFi.begin(ssid);

  Serial.print("Waiting on wifi ");
  int sanity = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("w");
    sanity++;
    if (sanity > 50) {
      Serial.println("ERROR: failed to connect to wifi");
      break;
    }
  }
  Serial.println("\nDone");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("snake")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  if (swSer.available() > 0) {
    String input = swSer.readStringUntil('\n');
    int index = input.indexOf(':');
    String s = input.substring(0, index);
    String msg = input.substring((index + 1));
    Serial.print("[" + s + "] Send message to WebSocket [" + msg +  "]");
    webSocket.sendTXT(s.toInt(), msg);
  }
}
