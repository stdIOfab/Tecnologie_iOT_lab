#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int REFRESH = 2000;
const byte BLED_PIN = 3;
byte bled_status = LOW;

const int B = 4275;
const int R0 = 100000;
const int T0 = 25;
const float KELVIN = 273.15;
const int TEMPSENSOR_PIN = 14;
int rawRead;
float R, temp;

String broker_address = "test.mosquitto.org";
int broker_port = 1883;

// Enough space for 1 SenML record (plus spare)
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument doc_snd(capacity);
DynamicJsonDocument doc_rec(capacity);

const String base_topic = "/tiot/group2";

//funzione che verifica a partire dal parametro passato sottoforma di stringa se si tratta di un float
bool isFloat(String tString){
  String tBuf;
  bool decPt = false;
  if(tString.charAt(0) == '+' || tString.charAt(0) == '-'){
    tBuf = &tString[1];
  }
  else{
    tBuf = tString;
  }  
  for(int x=0;x<tBuf.length();x++)
  {
    if(tBuf.charAt(x) == '.') {
      if(decPt){
        return false;
      }
      else{
        decPt = true; 
      } 
    }    
    else if(tBuf.charAt(x) < '0' || tBuf.charAt(x) > '9'){
      return false;
    }
  }
  return true;
}

float readTemp()
{
  rawRead = analogRead(TEMPSENSOR_PIN);
  
  R = (1023.0/rawRead)-1.0;
  R = R0*R;
  temp = 1.0/((log(R/R0)/B)+(1/(KELVIN+T0)))-KELVIN;
  return temp;
}

void callback(char* topic, byte* payload, unsigned int length) {
  DeserializationError err = deserializeJson(doc_rec, (char*) payload);
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  if(doc_rec["e"][0]["n"] == "led") {
      if(doc_rec["e"][0]["v"] == 1){
        bled_status = HIGH;
        Serial.println("Led Acceso.");
      }else if(doc_rec["e"][0]["v"] == 0){
        bled_status = LOW;
        Serial.println("Led spento.");
      }else{
    Serial.println("Errore, il valore del LED ricevuto dal file JSON non e' stato riconosciuto");
  }
  digitalWrite(BLED_PIN, bled_status); //Il LED anche in caso di errore nel file json rimane nello stato precedente al comando errato
  }else{
    Serial.println("Errore nel file JSON");
  }
}

WiFiClient wifi;
PubSubClient client(broker_address.c_str(), broker_port, callback, wifi);

int status = WL_IDLE_STATUS;


void reconnect() {
  // Loop until connected
  while (client.state() != MQTT_CONNECTED) {
    if (client.connect("TiotGroup2")) {
      client.subscribe((base_topic + String("/led")).c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(REFRESH);
    }
  }
}

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
  pinMode(BLED_PIN, OUTPUT);
  digitalWrite(BLED_PIN, bled_status);

  Serial.begin(9600);
  while(!Serial);
  Serial.println("Monitor Seriale inizializzato.");
  while(status != WL_CONNECTED){
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  status = WiFi.begin(ssid, pass);
  delay(10000);
  }

  Serial.print("Connected with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if(client.state() != MQTT_CONNECTED) {
    reconnect();
  }
  //publisher
  String body = senMlEncode(readTemp(), "C");
  client.publish((base_topic + String("/temperature")).c_str(), body.c_str());
  //subscriber
  client.loop();
  delay(REFRESH);
}
