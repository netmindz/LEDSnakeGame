// Use single controller setup where LEDs connected to ESP being used to run game as well as wifi control

#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#elif
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
//#include <Hash.h>
#include "HashMap.h"

#define SNAKE_CONTROL_WEBSOCKET

#include "wifi.h"
// Create file with the following
// *************************************************************************
// #define SECRET_SSID "";  /* Replace with your SSID */
// #define SECRET_PSK "";   /* Replace with your WPA2 passphrase */
// *************************************************************************
const char ssid[] = SECRET_SSID;
const char passphrase[] = SECRET_PSK;


WebSocketsServer webSocket = WebSocketsServer(81);
AsyncWebServer server(80);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length); // implemented in library

void controlSetup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

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

  leds[0] = CRGB::Blue;
  FastLED.show();

}

void controlLoop() {
  webSocket.loop();
}
