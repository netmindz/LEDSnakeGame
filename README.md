# LEDSnakeGame

Hey folks, been doing a bit more work on my version of snake, it should work on any size of matrix using regular FastLED and support for SmartMatrix/SmartLED Shield too, including the ability to zoom the display for when using high density panels like 64x64 p2.5

It can be used on any microcontroller by using the Serial Monitor as input with the game starting as soon as their is input. It can be used on ESP32 where you can play multi-player games over WebSocket where as each player connects they join the game supporting 4 players at the moment but can scale higher, just need to choose colors that are clearly distinguishable for the different players and possibly tweak the max websockets limit. You can also use dual-controller mode where you load the main GameSnake onto the controller with the LEDs and load the GameWebSocketServer onto an ESP8266 an connect a to the Serial1 port on the LED controlling controller (note: Serial1, not Serial!)

I need to try and get few videos of it running, but it's a little tricky to get other player to help test - though I did get a few work colleagues to test via Zoom :)
