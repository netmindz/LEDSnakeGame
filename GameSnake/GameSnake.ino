#define BRIGHTNESS      150
#define DELAY_DEFAULT   600 // Starting game speed (delay ms between frames)
#define DELAY_MIN       10 // Max game speed

#include <FastLED.h>

#ifdef __MK64FX512__ // Teensy 3.5 + SmartMatrixShield
  #include "smartmatrixControl.h"
#else
  #include "default.h"
#endif

uint16_t XY( uint8_t x, uint8_t y)
{
  if ( x >= kMatrixWidth) return -1;
  if ( y >= kMatrixHeight) return -1;

  uint16_t i;

  if ( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if ( kMatrixSerpentineLayout == true) {
    if ( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }

  return i;
}

#include "snake.h"

#ifdef ESP32
#include "control_esp.h"
#else
#include "control_teensy.h"
#endif

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup");

  ledSetup();

  leds[0] = CRGB::Red;
  FastLED.setBrightness(BRIGHTNESS);
  ledLoop();
  delay(5000);
  controlSetup();
}

boolean serialStart = false;
int incomingByte = 0;
void loop() {
  if (Serial.available() > 0) {
    if (!serialStart) {
      snakes[0].init(playerColors[0]);
      serialStart = true;
    }
    incomingByte = Serial.read();
    snakes[0].input(incomingByte);
  }
  controlLoop();
  EVERY_N_MILLISECONDS( snakes[0].getDelay() ) { // TODO: avg of all players?
    for (int s = 0; s < MAX_SNAKES; s++) {
      snakes[s].frame();
    }
    ledLoop();
    for (int s = 0; s < MAX_SNAKES; s++) {
      snakes[s].frameClear();
    }
  }
}
