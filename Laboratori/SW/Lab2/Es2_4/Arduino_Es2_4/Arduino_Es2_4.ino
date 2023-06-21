#include <WiFiNINA.h>
#include "arduino_secret.h"
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <Scheduler.h>

#define BUFFER_SIZE 10000

char server_address[] = "172.20.10.3"; //indirizzo del resource catalog
int server_port = 8080;
const int LOCAL_SERVER_PORT = 80;
const int REFRESH = 2000;
const int lease_refresh = 100; //secondi

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;
//GET dir
String kPath = "/";

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiClient wifi;
WiFiServer server(LOCAL_SERVER_PORT);
HttpClient http = HttpClient(wifi, server_address, server_port);

int status = WL_IDLE_STATUS;

bool system_ok = false;

String deviceSubscription;
String shortDeviceSubscription;

const int B = 4275;
const int R0 = 100000;
const int T0 = 25;
const float KELVIN = 273.15;
const int TEMPSENSOR_PIN = 14;
int rawRead;
float R, temp;

float readTemp()
{
  rawRead = analogRead(TEMPSENSOR_PIN);
  
  R = (1023.0/rawRead)-1.0;
  R = R0*R;
  temp = 1.0/((log(R/R0)/B)+(1/(KELVIN+T0)))-KELVIN;
  return temp;
}

const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument doc_snd(capacity);
DynamicJsonDocument doc_rec(capacity);

String senMlEncode(String type, float val, String measureUnit) {
  doc_snd.clear();
  if(type == "temperature"){
    doc_snd["bn"] = "ArduinoGroup2";
    doc_snd["e"][0]["n"] = "temperature";
    doc_snd["e"][1]["t"] = int(millis()/1000);
    doc_snd["e"][2]["v"] = val;
    doc_snd["e"][3]["u"] = measureUnit;
  }else if(type == "subscription"){
    String endpoints = "GET " + (String) WiFi.localIP() + ":" + (String) LOCAL_SERVER_PORT + "/temperature/";
    doc_snd["id"] = "ArduinoGroup2";
    doc_snd["endpoints"] = "GET ";
    doc_snd["availableRes"] = "temperature";
  }
  String output;
  serializeJson(doc_snd, output);
  return output;
}

void printResponse(WiFiClient client, int code, String body) {
  client.println("HTTP/1.1 " + String(code));
  if (code == 200) {
    client.println("Content-type: application/json; charset=utf-8");
    client.println();
    client.println(body);
  }else{
    client.println();
  }
}

void process(WiFiClient client) {
  String req_type = client.readStringUntil(' '); //definisce se si tratta di una get o altro
  req_type.trim(); //rimuove eventuali \n o altri caratteri speciali
  String url = client.readStringUntil(' ');
  url.trim();
  if(req_type == "GET"){
    if(url.startsWith("/temperature/"))
    printResponse(client, 200, senMlEncode("temperature", readTemp(), "C"));
  }else {
    printResponse(client, 400, "Bad Request, only GET implemented");
  }
}

void renewSubscription(){
  String body = senMlEncode("subscription", 0, "");
  http.beginRequest();
  http.post(shortDeviceSubscription);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("Content-Length", body.length());
  http.beginBody();
  http.print(body);
  http.endRequest();
  int retCode = http.responseStatusCode();
  Serial.println(retCode);
  yield();
  delay(lease_refresh*1000);
  return;
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Seriale inizializzata.");
  while (status != WL_CONNECTED) {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  status = WiFi.begin(ssid, pass);
  delay(10000);
  }

  Serial.print("Connected with IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Server started");

  //Il device interroga il catalog per conoscere a quale indirizzo puo' mandare le sue iscrizioni
  char buffer[BUFFER_SIZE];
  int bufferIndex = 0;
  int err;
  err = http.get("/");
  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println("Body returned follows:");

        unsigned long timeoutStart = millis();
        char c;

        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                buffer[bufferIndex++] = http.read();
               
                bodyLen--;
                timeoutStart = millis();
            }
            else
            {
                delay(kNetworkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  buffer[bufferIndex]= '\0';
  DeserializationError er = deserializeJson(doc_rec, (char*) buffer);
  if (er) {
    Serial.print(("deserializeJson() failed with code "));
    Serial.println(er.c_str());
  }else{
    String temp = doc_rec["subscriptions"]["REST"]["device"];
    deviceSubscription = temp;
    for(int i = 0; i < deviceSubscription.length(); i++){
      if(deviceSubscription[i] == '/' && i > 8){
        shortDeviceSubscription = deviceSubscription.substring(i);
        break;
      }
    }
    Serial.println(deviceSubscription);
    Serial.println(shortDeviceSubscription);
  }
  if(deviceSubscription == NULL){
    Serial.println("An error occurred while querying the catalog... the program will be stopped.");
  }else{
    system_ok = true;
  }
  Scheduler.startLoop(renewSubscription);
}

void loop() {
  if(system_ok){
    WiFiClient client = server.available();
    if (client) {
      process(client);
      client.stop();
    }
  }
  delay(REFRESH);
}
