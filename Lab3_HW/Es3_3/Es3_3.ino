#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

WiFiClient wifi;
PubSubClient client(broker_address.c_str(), broker_port, callback, wifi);

String broker_address = "test.mosquitto.org";
int broker_port = 1883;

// Enough space for 1 SenML record (plus spare)
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument doc_snd(capacity);
DynamicJsonDocument doc_rec(capacity);

const String base_topic = "/tiot/O";

void callback(char* topic, byte* payload, unsigned int length) {
  DeserializationError err = deserializeJson(doc_rec, (char*) payload);
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  if (doc_rec["e"][0]["n"] == "led") {
    if (doc_rec["e"][0]["v"] == 1) {

    }
  }
}

void reconnect() {
  // Loop until connected
  while (client.state() != MQTT_CONNECTED) {
    if (client.connect("TiotGroup0")) {
      client.subscribe((base_topic + String("/led")).c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String senMlEncode(String res, float v, String unit) {
  doc_snd.clear();
  doc_snd["bn"] = "ArduinoGroup";
  doc_snd["e"][0]["t"] = int(millis()/1000);
  // etc...
  String output;
  serializeJson(doc_snd, output);
  return output;
}

void setup() {

}

void loop() {
  if(client.state() != MQTT_CONNECTED) {
    reconnect();
  }
  //read sensor and create json message body...

  client.publish((base_topic + String("/temperature")).c_str(), body.c_str()) ;
  client.loop();
}
