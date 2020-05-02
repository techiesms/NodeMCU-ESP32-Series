#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif


#define LED1 15
#define LED2 12

char webpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>
<body>

<center>
<h1>My Home Automation</h1>

<h3> LED 1 </h3>
<button onclick="window.location = 'http://'+location.hostname+'/led1/on'">On</button><button onclick="window.location = 'http://'+location.hostname+'/led1/off'">Off</button>
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

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
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
}


void loop(void)
{
}
