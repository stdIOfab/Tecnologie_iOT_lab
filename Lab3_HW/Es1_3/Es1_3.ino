#include <WiFiNINA.h>
#include "arduino_secret.h"
#include <ArduinoJson.h>
#include <math.h>

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

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int LED_PIN = A2;
int status = WL_IDLE_STATUS;

WiFiServer server(80);

void printResponse(WiFiClient client, int code, String body);

const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument doc_snd(capacity);

String senMlEncode(String encodeType, float val, String measureUnit) {
  doc_snd.clear();
  doc_snd["bn"] = "ArduinoGroup";
  if (encodeType == "led") {
    doc_snd["e"][0]["n"] = "led";
    doc_snd["e"][1]["t"] = int(millis()/1000);
    doc_snd["e"][2]["v"] = val;
    doc_snd["e"][3]["u"] = "null";
  } else if (encodeType == "temperature") {
    doc_snd["e"][0]["n"] = "temperature";
    doc_snd["e"][1]["t"] = int(millis()/1000);
    doc_snd["e"][2]["v"] = val;
    doc_snd["e"][3]["u"] = measureUnit;
  }
  String output;
  serializeJson(doc_snd, output);
  return output;
} 

void process(WiFiClient client) {
  String req_type = client.readStringUntil(' ');
  req_type.trim();
  String url = client.readStringUntil(' ');
  url.trim();
  if (url.startsWith("/led/")) {
    String led_val = url.substring (5);
    Serial.println(led_val);
    if (led_val == "0" | led_val == "1") {
      int int_val = led_val.toInt();
      digitalWrite(LED_PIN, int_val);
      printResponse(client, 200, senMlEncode("led", int_val, ""));
    } else {
      printResponse(client, 400, "");
    }
  }else if(url.startsWith("/temperature/")) {
    printResponse(client, 200, senMlEncode("temperature", readTemp(), "Cel"));
  }else{
    printResponse(client, 404, "");    
  }
  }

void printResponse(WiFiClient client, int code, String body){
  client.println("HTTP/1.1 " + String(code) );
  if (code == 200) {
    client.println("Content-type: application/json; charset=utf-8");
    client.println(); //mandatory blank line
    client.println(body); //the response body
  } else {
    client.println();
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
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
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    process(client);
    client.stop();
  }
  delay (50);
}
