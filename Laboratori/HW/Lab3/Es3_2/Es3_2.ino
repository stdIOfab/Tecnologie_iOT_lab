#include <WiFiNINA.h>
#include "arduino_secret.h"
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>

char server_address[] = "172.20.10.4";
int server_port = 8080;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, server_address, server_port);

int status = WL_IDLE_STATUS;

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

String senMlEncode(float val, String measureUnit) {
  doc_snd.clear();
  doc_snd["bn"] = "ArduinoGroup2";
  doc_snd["e"][0]["n"] = "temperature";
  doc_snd["e"][1]["t"] = int(millis()/1000);
  doc_snd["e"][2]["v"] = val;
  doc_snd["e"][3]["u"] = measureUnit;
  String output;
  serializeJson(doc_snd, output);
  return output;
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

}

void loop() {
  String body = senMlEncode(readTemp(), "C");
  client.beginRequest();
  client.post("/log");
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", body.length());
  client.beginBody();
  client.print(body);
  client.endRequest();
  int ret = client.responseStatusCode();
  Serial.println(ret);
  delay(8000);
}
