#define MAX_SNAKES 4
#define SERIAL_DEBUG Serial


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      SERIAL_DEBUG.printf("[%u] Disconnected!\n", num);
      serialOut.printf("%u:e\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        SERIAL_DEBUG.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        if (num > (MAX_SNAKES - 1)) {
          Serial.println("MAX SNAKES!!");
          webSocket.sendTXT(num, "Sorry, player count exceeded");
          //webSocket.disconnect(num);
          return;
        }
        serialOut.printf("%u:s\n", num);
      }
      break;
    case WStype_TEXT:
      SERIAL_DEBUG.printf("[%u] get Text: %s\n", num, payload);
      serialOut.printf("%u:%s\n", num, payload);
      break;
    case WStype_BIN:
      SERIAL_DEBUG.printf("[%u] get binary length: %u\n", num, length);
//      hexdump(payload, length);
      break;
  }

}

