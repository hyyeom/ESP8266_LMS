#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>
#include <SPI.h>
#include <LittleFS.h>

// Replace with your network credentials
const char *ssid = "N704";
const char *password = "0123456789";
//const char *ssid = "2lms5129";
//const char *password = "qazwsx#@";
IPAddress ip(192, 168, 0, 101);
IPAddress dns(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
String myKey = "0";
const int ledPin = 15; //switch  15
const int ledPin1 = 2; //On board LED HIGH->OFF LOW->ON
String ledState;
AsyncWebServer server(80);
AsyncEventSource events("/events");
JSONVar readings;
// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

Adafruit_AM2320 AM2320 = Adafruit_AM2320();
void initAM2320()
{
  AM2320.begin();
}
void initLittleFS()
{
  if (!LittleFS.begin())
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}
// Initialize WiFi
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (!WiFi.config(ip, gateway, subnet, dns))
  {
    Serial.println("Static IP Configuration Failed");
  }
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
String myState()
{
  if (digitalRead(ledPin))
  {
    ledState = "OFF";
  }
  else
  {
    ledState = "ON";
  }
  return ledState;
}
String processor(const String &var)
{
  if (var == "STATE")
  {
    if (digitalRead(ledPin))
    {
      ledState = "OFF";
    }
    else
    {
      ledState = "ON";
    }
    return ledState;
  }
  return String();
}
String getSensorReadings()
{
  readings["temperature"] = String(AM2320.readTemperature());
  readings["humidity"] = String(AM2320.readHumidity());
  readings["swstate"] = String(myState());
  readings["key"] = String(myKey);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}
void winkLED(int n)
{
  for (int i = 0; i < n; i++)
  {
    digitalWrite(ledPin1, LOW);
    delay(200);
    digitalWrite(ledPin1, HIGH);
    delay(200);
  }
}
void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(ledPin1, HIGH);
  Serial.println("Start!!");
  initAM2320();
  Serial.println("AM2320");
  initWiFi();
  initLittleFS();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              request->send(LittleFS, "/index.html", "text/html");
              Serial.println("send index.html");
            });

  server.serveStatic("/", LittleFS, "/");

  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              request->send(LittleFS, "/control.html", "text/html");
              Serial.println("send control.html");
            });
  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String json = getSensorReadings();
              request->send(200, "application/json", json);
              Serial.println(json);
              json = String();
              myKey = "0";
            });
  server.on("/readData", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              digitalWrite(ledPin1, LOW);
              String json = getSensorReadings();
              request->send(200, "application/json", json);
              Serial.println(json);
              json = String();
              myKey = "0";
              delay(20);
              digitalWrite(ledPin1, HIGH);
            });
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              //int pCnt = request->params();
              AsyncWebParameter *p = request->getParam(0);
              String pname = p->name();
              String rpass = p->value();
              if (rpass == "5093")
              {
                myKey = "5093";
              }
              else
              {
                myKey = "0";
              }
            });

  server.on("/onoff", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (digitalRead(ledPin))
              {
                digitalWrite(ledPin, LOW);
              }
              else
              {
                digitalWrite(ledPin, HIGH);
              }
              String json = getSensorReadings();
              request->send(200, "application/json", json);
              json = String();
            });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "The content you are looking for was not found."); });
  server.begin();

  winkLED(3);
}

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping", NULL, millis());
    events.send(getSensorReadings().c_str(), "new_readings", millis());
    lastTime = millis();
  }
}
