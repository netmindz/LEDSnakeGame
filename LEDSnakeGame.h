#include <WebSocketsServer.h>
#include <HashMap.h>
#include <ArduinoJson.h>

#define MAX_SNAKES 4
#define SERIAL_DEBUG Serial

#define DELAY_DEFAULT   300 // Starting game speed (delay ms between frames)
#define DELAY_MIN       100 // Max game speed


CRGB playerColors[MAX_SNAKES] = {CRGB::Blue, CRGB::DarkMagenta, CRGB::Yellow, CRGB::OrangeRed};



class Point {

  public:
    int x;
    int y;

  public:
    Point() {
    }

    void setXY(int newX, int newY) {
      x = newX;
      y = newY;
    }
};

class GameSnake {

    char dir;
    int x;
    int y;
    int l;
    int fruit;
    Point tail[200];
    int d = DELAY_DEFAULT; // TODO need to make speed per snake
    boolean started = false;
    boolean isDead  = false;
    int deathPulse = 0;
    CRGB colorH;
    CRGB colorT;

  public:
    GameSnake() {
    }

    void init(CRGB color) {
      started = true;
      colorH = color;
      colorT = CRGB(color.r, color.g, color.b);
      colorT = colorT.fadeLightBy(200);
      reset();
    }

    void reset() {
      isDead = false;
      leds[XY(x, y)] = CRGB::Black;
      renderTail(CRGB::Black);
      x = random(4, (kMatrixWidth - 4));
      y = random(4, (kMatrixHeight - 4));
      if (random(0, 1) == 1) {
        dir = 'L';
      }
      else {
        dir = 'R';
      }
      l = 0;
      d = DELAY_DEFAULT;
      Serial.println("Go!");
      newFruit();
    }

    void frame() {
      if (!started) {
        return;
      }
      if (isDead) {
        if (deathPulse > 0) {
          if ( deathPulse & 0x01) {
            renderTail(CRGB::Red);
          }
          else {
            renderTail(CRGB::Black);
          }
          deathPulse--;
        }
        else {
          leds[fruit] = CRGB::Black; // TODO: not sure about this
          reset();
        }
        return;
      }

      renderTail(colorT);

      switch (dir) {
        case 'U':
          y++;
          break;
        case 'D':
          y--;
          break;
        case 'R':
          x--;
          break;
        case 'L':
          x++;
          break;
      }

      CRGB currentColor = leds[XY(x, y)];

      if (x > (kMatrixWidth - 1)) {
        die();
      }
      else if (x < 0) {
        die();
      }
      else if (y > (kMatrixHeight - 1)) {
        die();
      }
      else if (y < 0) {
        die();
      }
      else if (currentColor.g != 0 && currentColor.r == 0 && currentColor.b == 0) {
        //        Serial.printf("Current g,r value (%i,%i) = %i %i\n", x, y, currentColor.g, currentColor.r);
        eat();
      }
      else if (currentColor.r != 0 || currentColor.g != 0 || currentColor.b != 0) {
        die();
      }

      leds[XY(x, y)] = colorH;
    }

    void frameClear() {
      if (!started) {
        return;
      }

      Point tailEnd = tail[l];
      leds[XY(tailEnd.x, tailEnd.y)] = CRGB::Black;

      if (!isDead) {
        // move tail values up by one
        for (int i = l; i >= 1; i--) {
          tail[i] = tail[(i - 1)];
        }

        Point h;
        h.setXY(x, y);
        tail[0] = h;
      }
    }

    void die() {
      Serial.println("Dead");
      isDead = true;
      deathPulse = 5;
    }

    void exit() {
      Serial.println("Player exit");
      renderTail(CRGB::Black);
      leds[fruit] = CRGB::Black;
      started = false;
    }

    void input(int c) {
      switch (c) {
        case 117:
          Serial.println("Up");
          if (dir != 'D') dir = 'U';
          break;
        case 100:
          Serial.println("Down");
          if (dir != 'U') dir = 'D';
          break;
        case 108:
          Serial.println("Left");
          if (dir != 'R') dir = 'L';
          break;
        case 114:
          Serial.println("Right");
          if (dir != 'L') dir = 'R';
          break;
        default:
          Serial.print("Unknown input ");
          Serial.println(c);
      }
    }

    void renderTail(CRGB color) {
      for (int i = 0; i < l; i++) {
        Point t = tail[i];
        leds[XY(t.x, t.y)] = color;
      }
    }


    void newFruit() {
      int i = fruit = XY(random(1, (kMatrixWidth - 1)), random(1, (kMatrixHeight - 1)));
      if (leds[i].r != 0 || leds[i].g != 0 || leds[i].b != 0) {
        Serial.println("Fruit inside, retry");
        newFruit();
      }
      else {
        Serial.println("Show new fruit");
        leds[i] = CRGB::Green;
        fruit = i;
      }
    }

    void eat() {
      l++;
      newFruit();
      d = d * 0.9;
      if ( d < DELAY_MIN) {
        d = DELAY_MIN;
      }
      Serial.printf("Reduce delay to %u", d);
    }

    int getDelay () {
      return d;
    }

    boolean isStarted() {
      return started;
    }
};

GameSnake snakes[MAX_SNAKES];
CreateHashMap (snakeMap, IPAddress, int, MAX_SNAKES);

#ifdef serialOut
void webSocketEventSerial(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

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
#endif

//#ifdef SNAKE_CONTROL_WEBSOCKET

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  int snakeIndex = 0;
  IPAddress ip = webSocket.remoteIP(num);
  int s = snakeMap[ip];
  switch (type) {

    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      snakes[s].exit();
      snakeIndex = s; // TODO: uses as next snake, but will fail if you had 3
      break;

    case WStype_CONNECTED:      {
        Serial.printf("[%u] Connection from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        if (snakeIndex > (MAX_SNAKES - 1)) {
          Serial.println("MAX SNAKES!!");
          webSocket.sendTXT(num, "Sorry, player count exceeded");
//          webSocket.disconnect(num);
          return;
        }
        // send message to client
        CRGB color = playerColors[snakeIndex];
        char rgbTxt[8];
        sprintf(rgbTxt, "#%02X%02X%02X", color.r, color.g, color.b);
        String msg = "Connected player = " + (String) (snakeIndex + 1) + " <span style=\"background: " + rgbTxt +  "\">&nbsp;&nbsp;</span>";
        Serial.println(msg);
        webSocket.sendTXT(num, msg);
        snakes[snakeIndex].init(playerColors[snakeIndex]);
        snakeMap[ip] = snakeIndex;
        snakeIndex++;

        // TODO: remove this and replace with proper tracking of active players
        leds[0] = CRGB::Black;
      }
      break;

    case WStype_TEXT: {
        Serial.printf("[%u] got Text: %s\n", num, payload);
        for ( size_t i = 0; i < length; i++ ) {
          snakes[s].input( payload[i] );
        }
        // send message to client
        //webSocket.sendTXT(num, "Sent command");
      }
      break;

    case WStype_BIN:
      Serial.printf("[%u] got binary length: %u\n", num, length);
      //      hexdump(payload, length);
      break;
  }

}
//#endif

int avgDelay() {
  int a = 0;
  int total = 0;
  for (int s = 0; s < MAX_SNAKES; s++) {
    if(snakes[s].isStarted()) {
      a++;
      total += snakes[s].getDelay();
    }
  }
  if(total == 0) {
    return DELAY_DEFAULT;
  }
  return (total / a);
}

void ledToWebsocket() {
  StaticJsonDocument<5000> gameStateJson; // TODO: check size and if should be dynamic or static
  gameStateJson["width"] = kMatrixWidth;
  gameStateJson["height"] = kMatrixHeight;
  JsonArray data = gameStateJson.createNestedArray("data");
  for(uint16_t i = 0; i < NUM_LEDS; i++) {
    if(leds[i].r != 0 || leds[i].g != 0 || leds[i].b != 0) {
      char colorBuffer[8];
      sprintf(colorBuffer, "#%02X%02X%02X", leds[i].r, leds[i].g, leds[i].b);
      JsonObject pixel = data.createNestedObject();
      pixel["i"] = i;
      pixel["c"] = colorBuffer;
    }
  }
  String output;
  serializeJson(gameStateJson, output);
  webSocket.broadcastTXT(output);
}


boolean serialStart = false;
int incomingByte = 0;

void playSnake() {
  if (Serial.available() > 0) {
    if (!serialStart) {
      leds[0] = CRGB::Black;
      snakes[0].init(playerColors[0]);
      serialStart = true;
    }
    incomingByte = Serial.read();
    snakes[0].input(incomingByte);
  }
  controlLoop();
  EVERY_N_MILLISECONDS_I(timingObj, DELAY_DEFAULT) { 
    timingObj.setPeriod(avgDelay());
    for (int s = 0; s < MAX_SNAKES; s++) {
      snakes[s].frame();
    }
    ledToWebsocket();
    ledLoop();
    for (int s = 0; s < MAX_SNAKES; s++) {
      snakes[s].frameClear();
    }
  }
}

const char snake_html[] PROGMEM = R"rawliteral(
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1, user-scaleable=no">
<title>Snake</title>
 <script type="text/javascript" src="http://code.jquery.com/jquery-latest.min.js"></script>
<script>
var websocket;
function start() {
	$("#controls").hide();

  var hostname = window.location.hostname;
  websocket = new WebSocket('ws://'+hostname+':81/');
  setStatus("Connecting " + hostname + " ...");
  $("#connect").hide();
  websocket.onopen = function(evt) { 
		  setStatus("Connected!");
		  console.log('websocket open');
		  setControlsVisible();
	  };
  websocket.onclose = function(evt) { setStatus("Lost connection :("); console.log('websocket close'); };
  websocket.onerror = function(evt) { setStatus("ERROR: " + evt); console.log(evt); $("#connect").show();};
  websocket.onmessage =function(event) { console.debug("WebSocket message received:", event); if(!event.data.startsWith("{")) { setStatus(event.data); } else { setData(event.data); } };
  
}

</script>
<style>
button {
	width: 8em;
	height: 8em;
}
</style>
</head>
<body> 
<center><h1>Snake</h1></center>

<div id="connect">
	<input type="button" value="Connect" onClick="start()">
</div>

<div id="status"></div>
<div id="controls">
  <canvas id="game-canvas"></canvas>
  <table>
		<tr>
			<td></td>
			<td><button id="up"></button></td>
			<td></td>
		</tr>
		<tr>
			<td><button id="left"></button></td>
			<td></td>
			<td><button id="right"></button></td>
		</tr>
		<tr>
			<td></td>
			<td><button id="down"></button></td>
			<td></td>
		</tr>
	</table>
	<!--	<button id="new">NEW Game</button> -->
	<p>You may also use the cursor keys (recommended on desktop)</p>
</div>
<style>
      canvas {
        border: 1px solid black;
        background-color: grey;
      }
</style>
<script>
	function setStatus(msg) {
		$("#status").html(msg);
	}

  function setData(msg) {
    var scale = 4;
    const canvas = document.getElementById("game-canvas");
    const state = JSON.parse(msg);
    canvas.width = state.width * scale;
    canvas.height = state.height * scale;
    if (canvas.getContext) {
        const ctx = canvas.getContext("2d");
        ctx.clearRect(0, 0, 300, 300); // clear canvas
        for(var n = 0; n < state.data.length; n++) {
          var pixel = state.data[n];
          ctx.fillStyle = pixel.c;
          var x = (pixel.i % state.width) * scale;
          var y = Math.floor(pixel.i / state.width) * scale;
          console.log(pixel);
          console.log("x=" + x + " y=" + y);
          ctx.fillRect(x, y, scale, scale);          
        }
    }
  }

	function setControlsVisible() {
		$("#connect").hide();
		$("#controls").show();
	}

function sendCmd(cmd) {
//	$("#status").innerHTML = cmd;
	console.log("Sending " + cmd);
	websocket.send(cmd);
}

$("#new").click(function(){
	sendCmd("n");
});		
	
$("#left").click(function(){
	sendCmd("l");
});	
$("#right").click(function(){
	sendCmd("r");	
});	
$("#up").click(function(){
	sendCmd("u");
});	
$("#down").click(function(){
	sendCmd("d");
});	


$( document ).ready(function(){
	console.info("Ready");

document.onkeydown = checkKey;

function checkKey(e) {

    e = e || window.event;

    if (e.keyCode == '38') {
        // up arrow
	sendCmd("u");
    }
    else if (e.keyCode == '40') {
        // down arrow
	sendCmd("d");
    }
    else if (e.keyCode == '37') {
       // left arrow
	sendCmd("l");
    }
    else if (e.keyCode == '39') {
       // right arrow
	sendCmd("r");
    }
    else {
	    console.log("Code = " + e.keyCode);
    }

}
	start();	
} );
					
					
					
</script>
</body>
</html>
)rawliteral";

void setupSnake() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // Route for root / web page
  server.on("/snake", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", snake_html);
  });
}


