#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266mDNS.h>
#endif
#include <WebSocketsServer.h>
// #include <Hash.h>

#ifdef ESP8266
SoftwareSerial serialOut(D3, D4); //, false);
#else
#define serialOut Serial2
#endif
WebSocketsServer webSocket = WebSocketsServer(81);

#include <LEDSnakeGame.h>

#include "wifi.h"
// Create file with the following
// *************************************************************************
// #define SECRET_SSID "";
// #define SECRET_PSK "";
// *************************************************************************
const char ssid[] = SECRET_SSID;
const char passphrase[] = SECRET_PSK;




void setup() {
  SERIAL_DEBUG.begin(115200);
  serialOut.begin(115200);

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
  webSocket.onEvent(webSocketEventSerial);
}

void loop() {
  webSocket.loop();
  if (serialOut.available() > 0) {
    String input = serialOut.readStringUntil('\n');
    int index = input.indexOf(':');
    String s = input.substring(0, index);
    String msg = input.substring((index + 1));
    Serial.print("[" + s + "] Send message to WebSocket [" + msg +  "]");
    webSocket.sendTXT(s.toInt(), msg);
  }
}
