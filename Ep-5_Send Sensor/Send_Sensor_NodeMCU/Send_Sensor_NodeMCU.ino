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

#include <ArduinoJson.h>
#include <Ticker.h> //https://github.com/sstaub/Ticker


#define LED1 13
#define LED2 12


void send_sensor();
Ticker timer;

char webpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>

<script>

var connection = new WebSocket('ws://'+location.hostname+':81/');

var button_1_status = 0;
var button_2_status = 0;
var sens_data = 0;
connection.onmessage = function(event){

  var full_data = event.data;
  console.log(full_data);
  var data = JSON.parse(full_data);
  sens_data = data.sens;
  document.getElementById("sensor_meter").value = sens_data;
  document.getElementById("sensor_value").innerHTML = sens_data;
}

function button_1_on()
{
   button_1_status = 1; 
  console.log("LED 1 is ON");
  send_data();
}

function button_1_off()
{
  button_1_status = 0;
console.log("LED 1 is OFF");
send_data();
}

function button_2_on()
{
   button_2_status = 1; 
  console.log("LED 2 is ON");
  send_data();
}

function button_2_off()
{
  button_2_status = 0;
console.log("LED 2 is OFF");
send_data();
}

function send_data()
{
  var full_data = '{"LED1" :'+button_1_status+',"LED2":'+button_2_status+'}';
  connection.send(full_data);
}


</script>
<body>

<center>
<h1>My Home Automation</h1>

<h3> LED 1 </h3>
<button onclick= "button_1_on()" >On</button><button onclick="button_1_off()" >Off</button>
<h3> LED 2 </h3>
<button onclick="button_2_on()">On</button><button onclick="button_2_off()">Off</button>
</center>

<div style="text-align: center;">
<h3>Proximity Sensor</h3><meter value="0" min="0" max="1" id="sensor_meter"> </meter><h3 id="sensor_value" style="display: inline-block;"> 2 </h3>


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
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload));
      Serial.println(message);

      
     DynamicJsonDocument doc(200);
    // deserialize the data
    DeserializationError error = deserializeJson(doc, message);
    // parse the parameters we expect to receive (TO-DO: error handling)
      // Test if parsing succeeds.
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  int LED1_status = doc["LED1"];
  int LED2_status = doc["LED2"];
  digitalWrite(LED1,LED1_status);
  digitalWrite(LED2,LED2_status);




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
  timer.attach(0.5,send_sensor);
  pinMode(D5,INPUT);

}


void loop(void)
{
 websockets.loop();
}

void send_sensor()
{
   int sens = digitalRead(D5);
  // JSON_Data = {"temp":t,"hum":h}"
  String JSON_Data = "{\"sens\":";
         JSON_Data += sens;
         JSON_Data += "}";
   Serial.println(JSON_Data);     
  websockets.broadcastTXT(JSON_Data);
}
