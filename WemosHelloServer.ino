#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>

#define EXTERNAL_LED_PIN D2

const char* ssid = "****";
const char* password = "****";
int internalLEDPin;
int internalLEDValue;
int externalLEDValue;

ESP8266WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, EXTERNAL_LED_PIN, NEO_GRB + NEO_KHZ800);

void handleRoot()
{
  server.send(200, "text/plain", "hello from the Wemos Mini D1 Pro!!!");
}

void handleUpload()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", serverIndex);
}

void handleUpdate()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
  ESP.restart();
}

void handleUpdate2()
{
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    Serial.setDebugOutput(true);
    WiFiUDP::stopAll();
    Serial.printf("Update: %s\n", upload.filename.c_str());
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if(!Update.begin(maxSketchSpace)){//start with max available size
      Update.printError(Serial);
    }
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
      Update.printError(Serial);
    }
  } else if(upload.status == UPLOAD_FILE_END){
    if(Update.end(true)){ //true to set the size to the current progress
      Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
    Serial.setDebugOutput(false);
  }
  yield();
}

void handleLED()
{
  for (uint8_t i=0; i<server.args(); i++)
  {
    if (server.argName(i) == "internal")
    {
      if (server.arg(i) == "off")
      {
        internalLEDValue = HIGH;
      }
      else if (server.arg(i) == "on")
      {
        internalLEDValue = LOW;
      }
      digitalWrite(internalLEDPin, internalLEDValue);
    }

    if (server.argName(i) == "external")
    {
      if (server.arg(i) == "red")
      {
        externalLEDValue = 1;
        pixels.setPixelColor(0, 100, 0, 0);
        pixels.show();
      }
      else if (server.arg(i) == "green")
      {
        externalLEDValue = 2;
        pixels.setPixelColor(0, 0, 100, 0);
        pixels.show();
      }
      else if (server.arg(i) == "yellow")
      {
        externalLEDValue = 3;
        pixels.setPixelColor(0, 100, 100, 0);
        pixels.show();
      }
      else if (server.arg(i) == "orange")
      {
        externalLEDValue = 4;
        pixels.setPixelColor(0, 200, 100, 0);
        pixels.show();
      }
      else if (server.arg(i) == "off")
      {
        externalLEDValue = 0;
        pixels.setPixelColor(0,0,0,0);
        pixels.show();
      }
    }
  }
  
  String message = "<html>\n<head>";
  message += "<style>\n";
  message += "table {\n";
  message += "  border-collapse : collapse;\n";
  message += "}\n";
  message += "table, th, td {border : 1px solid black;}\n";
  message += "th {background-color : lightblue;}\n";
  message += "td {text-align : center;}\n";
  message += "</style>\n";
  message += "</head><body>";
  message += "LED directory\n\n";
  message += "<table><tr><th>LED</th><th>State</th></tr>";
  message += "<tr><td>Internal</td><td><a href=\"/LED?internal=";
  message += internalLEDValue == HIGH ? "on" : "off";
  message += "\">";
  message += internalLEDValue == HIGH ? "Off" : "On";
  message += "</a></td></tr>";
  message += "<tr><td>External</td><td><a href=\"/LED?external=off\">";
  message += externalLEDValue == 0 ? "<b>":"";
  message += "Off";
  message += externalLEDValue == 0 ? "</b>":"";
  message += "</a>&nbsp;<a href=\"/LED?external=red\">";
  message += externalLEDValue == 1 ? "<b>":"";
  message += "Red";
  message += externalLEDValue == 1 ? "</b>":"";
  message += "</a>&nbsp;<a href=\"/LED?external=green\">";
  message += externalLEDValue == 2 ? "<b>":"";
  message += "Green";
  message += externalLEDValue == 2 ? "</b>":"";
  message += "</a>&nbsp;<a href=\"/LED?external=yellow\">";
  message += externalLEDValue == 3 ? "<b>":"";
  message += "Yellow";
  message += externalLEDValue == 3 ? "</b>":"";
  message += "</a>&nbsp;<a href=\"/LED?external=orange\">";
  message += externalLEDValue == 4 ? "<b>":"";
  message += "Orange";
  message += externalLEDValue ==4 ? "</b>":"";
  message += "</a></td></tr>";
  message += "</table>";
  message += "<br>";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  message += "</body>";
  message += "</html>";

  for (uint8_t i=0; i<server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(200, "text/html", message);
}

void handleNotFound() 
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i=0; i<server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  internalLEDPin = D4;
  internalLEDValue = HIGH;  // This is off
  pinMode(internalLEDPin, OUTPUT);
  digitalWrite(internalLEDPin, internalLEDValue);

  externalLEDValue = 0;
  pixels.begin();
  pixels.setPixelColor(0, 0, 0, 0); // Turn off the external LED
  pixels.show();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/inline", []()
  {
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/LED", handleLED);
  server.on("/update", HTTP_POST, handleUpdate, handleUpdate2);
  server.on("/upload", handleUpload);
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
}
