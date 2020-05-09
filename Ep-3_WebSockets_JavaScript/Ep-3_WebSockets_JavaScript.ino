#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif

#include <WebSocketsServer.h>



#define LED1 13
#define LED2 12

char webpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>

<script>

var connection = new WebSocket('ws://'+location.hostname+':81/');

function button_1_on()
{
  console.log("LED 1 is ON");
  connection.send("LED 1 is ON");
}

function button_1_off()
{
console.log("LED 1 is OFF");
connection.send("LED 1 is OFF");
}


</script>
<body>

<center>
<h1>My Home Automation</h1>

<h3> LED 1 </h3>
<button onclick= "button_1_on()" >On</button><button onclick="button_1_off()" >Off</button>
<h3> LED 2 </h3>
<button onclick="window.location = 'http://'+location.hostname+'/led2/on'">On</button><button onclick="window.location = 'http://'+location.hostname+'/led2/off'">Off</button>

</center>
</body>
</html>

)=====";

// ipaddress/led1/on
//ipaddress/led1/off

// ipaddress/led2/on
//ipaddress/led2/off
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80); // server port 80
WebSocketsServer websockets(81);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        websockets.sendTXT(num, "Connected from server");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload));
      Serial.println(message);

      if(message == "LED 1 is OFF"){
        digitalWrite(LED1,LOW);
      }

      if(message == "LED 1 is ON"){
        digitalWrite(LED1,HIGH);
      }
      


  }
}

void setup(void)
{
  
  Serial.begin(115200);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  
  WiFi.softAP("techiesms", "");
  Serial.println("softap");
  Serial.println("");
  Serial.println(WiFi.softAPIP());


  if (MDNS.begin("ESP")) { //esp.local/
    Serial.println("MDNS responder started");
  }



  server.on("/", [](AsyncWebServerRequest * request)
  { 
   
  request->send_P(200, "text/html", webpage);
  });

   server.on("/led1/on", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    digitalWrite(LED1,HIGH);
  request->send_P(200, "text/html", webpage);
  });

  server.onNotFound(notFound);

  server.begin();  // it will start webserver
  websockets.begin();
  websockets.onEvent(webSocketEvent);

}


void loop(void)
{
 websockets.loop();
}
