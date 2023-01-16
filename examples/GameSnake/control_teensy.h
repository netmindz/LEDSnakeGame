// Control based on serial input feed by connecting to ESP8266 or similar

void controlSetup() {
  Serial1.begin(115200);
  leds[0] = CRGB::Blue;
}

void controlLoop() {
  if (Serial1.available() > 0) {
    String input = Serial1.readStringUntil('\n');
    int index = input.indexOf(':');
    int s = input.substring(0, index).toInt();
    char cmd = input.substring((index + 1)).charAt(0);
    Serial.println("Input: [" + input + "] s=" + s + " cmd=" + cmd);

    if (cmd == 115) {
      leds[0] = CRGB::Black;
      CRGB color = playerColors[s];
      char rgbTxt[8];
      sprintf(rgbTxt, "#%02X%02X%02X", color.r, color.g, color.b);
      String msg = "Connected player = " + (String) (s + 1) + " <span style=\"background: " + rgbTxt +  "\">&nbsp;&nbsp;</span>";
      Serial1.println((String) s + ":" + msg);
      Serial.println("Start snake " + (String) s);
      snakes[s].init(color);
    }
    else if (cmd == 101) {
      Serial1.println((String) s + ":Exit snake " + (String) s);
      Serial.println("Exit snake " + (String) s);
      snakes[s].exit();
    }
    else {
      snakes[s].input(cmd);
    }
  }
}
