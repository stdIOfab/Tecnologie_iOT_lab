#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secret.h"
#include <ArduinoHttpClient.h>
#include <MBED_RPi_Pico_TimerInterrupt.h>

#define BUFFER_SIZE 10000 //dimensione del buffer per ricevere i dati di GET dal Catalog

const String ID_DEVICE = "TIoT2"; //identifica univocamente il dispositivo
const int RENEW_SUBSCRIPTION_TIME = 60; // secondi che intercorrono tra una registrazione e il successivo rinnovo
long int lastRenewal = -RENEW_SUBSCRIPTION_TIME*1000;
String subscriptionForm; //contenuto sottoforma di json testuale della richiesta di subscription

MBED_RPI_PICO_Timer ITimer1(1); //timer per il rinnovo della subscription

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

String broker_address; //indirizzo IP del broker
int broker_port; //porta del broker
String subscription_topic;
char server_address[] = "172.20.10.3"; //indirizzo del resource catalog
int server_port = 8080; //porta del resource catalog

WiFiClient wifi;
HttpClient http = HttpClient(wifi, server_address, server_port);
PubSubClient client(wifi);

//variabili di attesa per la GET in fase iniziale per il collegamento con il catalog
const int kNetworkTimeout = 30*1000;
const int kNetworkDelay = 1000;
//indirizzo del catalog alla quale far riferimento
String kPath = "/";

// Enough space for 1 SenML record (plus spare)
const int capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(8) + 100;
DynamicJsonDocument doc_snd(capacity);
DynamicJsonDocument doc_rec(capacity);

const String base_topic = "/tiot/2/device/";

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

//genera il contenuto JSON sottoforma testuale per la richiesta di subscription
String subFormGenerator(){
  doc_snd.clear();
  JsonObject root = doc_snd.to<JsonObject>();
  root["id"] = ID_DEVICE;
  JsonObject endpnts = root.createNestedObject("endpoints");
  JsonObject MQTTobj = endpnts.createNestedObject("MQTT");
  JsonObject RESTobj = endpnts.createNestedObject("REST");
  JsonArray PUBLISHarray = MQTTobj.createNestedArray("PUBLISH");
  JsonArray SUBSCRIBEarray = MQTTobj.createNestedArray("SUSCRIBE");
  JsonArray POSTarray = RESTobj.createNestedArray("POST");
  JsonArray GETarray = RESTobj.createNestedArray("GET");
  SUBSCRIBEarray.add(base_topic + ID_DEVICE + "/temperature");
  PUBLISHarray.add(base_topic + ID_DEVICE + "/led");
  root["timestamp"] = millis();
  JsonArray avlblRs = root.createNestedArray("availableRes");
  avlblRs.add("temperature");
  avlblRs.add("led");
  
  String output;
  serializeJson(doc_snd, output);
  return output;
}

//lettura della temperatura
float readTemp()
{
  rawRead = analogRead(TEMPSENSOR_PIN);
  
  R = (1023.0/rawRead)-1.0;
  R = R0*R;
  temp = 1.0/((log(R/R0)/B)+(1/(KELVIN+T0)))-KELVIN;
  return temp;
}

//funzione di callback per la subscribe in seguito ad un messaggio pubblicato
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

int status = WL_IDLE_STATUS;

//funzione di codifica del senML utilizzata per le operazioni di publish
String senMlEncode(float val, String measureUnit) {
  doc_snd.clear();
  doc_snd["bn"] = ID_DEVICE;
  doc_snd["e"][0]["n"] = "temperature";
  doc_snd["e"][1]["t"] = int(millis()/1000);
  doc_snd["e"][2]["v"] = val;
  doc_snd["e"][3]["u"] = measureUnit;
  String output;
  serializeJson(doc_snd, output);
  return output;
}

void reconnect(){
  // Loop until connected
  while (client.state() != MQTT_CONNECTED) {
    if (client.connect("TiOTArduino2")) {
      client.subscribe((base_topic + ID_DEVICE + String("/led")).c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(REFRESH);
    }
  }
}

//nella fase di setup interroga il catalog per ricevere i parametri di funzionamento e successivamente registrarsi
void readCatalog(){
  char buffer[BUFFER_SIZE];
  int bufferIndex = 0;
  int err;
  err = http.get("/");
  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0){
      Serial.print("Got status code: ");
      Serial.println(err);

      err = http.skipResponseHeaders();
      if (err >= 0){
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println("Body returned follows:");

        unsigned long timeoutStart = millis();
        char c;

        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) ){
            if (http.available()){
                buffer[bufferIndex++] = http.read();
               
                bodyLen--;
                timeoutStart = millis();
            }else{
                delay(kNetworkDelay);
            }
        }
      }else{
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }else{    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }else{
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  buffer[bufferIndex]= '\0';
  DeserializationError er = deserializeJson(doc_rec, (char*) buffer);
  if (er) {
    Serial.print(("deserializeJson() failed with code "));
    Serial.println(er.c_str());
  }else{
    broker_address = doc_rec["subscriptions"]["MQTT"]["device"]["hostname"].as<String>();
    broker_port = doc_rec["subscriptions"]["MQTT"]["device"]["port"].as<String>().toInt();
    subscription_topic = doc_rec["subscriptions"]["MQTT"]["device"]["topic"].as<String>();
    Serial.println(broker_address);
    Serial.println(broker_port);
  }
}

//rinnnovo della subscription al catalog
void renewSubscription(){
  if(client.publish(subscription_topic.c_str(), subscriptionForm.c_str())){
    Serial.println("Subscription renew completed!");
    lastRenewal = millis();
  }else{
    Serial.println("Subscription renew failed... new try later...");
  }
}

void MQTTPublisher(){
  String body = senMlEncode(readTemp(), "C");
  bool status = client.publish((base_topic + ID_DEVICE + String("/temperature")).c_str(), body.c_str());
  if(status){
    Serial.print("Publish succeed at topic: ");
    Serial.println(base_topic + ID_DEVICE + String("/temperature"));
  }else{
    Serial.println("Publish failed");
  }
}

void setup() {
  //definisce gli output
  pinMode(BLED_PIN, OUTPUT);
  digitalWrite(BLED_PIN, bled_status);

  //inizializazione del monitor seriale
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Monitor Seriale inizializzato.");
  //connessione alla rete WIFI
  while(status != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000); //attende 10 secondi tra un tentativo e l'altro
  }
  Serial.print("Connected with IP Address: ");
  Serial.println(WiFi.localIP());

  //invia una richiesta al catalog per potersi iscrivere al servizio MQTT
  readCatalog();
  if(broker_address == NULL || broker_port == NULL){
    Serial.println("An error occurred while querying the catalog... the program will try again in 4 seconds...");
    delay(4000);
    readCatalog();
  }else{
    client.setServer(broker_address.c_str(), broker_port);
    client.setCallback(callback);
    subscriptionForm = subFormGenerator();
    Serial.println(subscriptionForm);
    Serial.println(subscription_topic);
  }
}

void loop() {
  if(client.state() != MQTT_CONNECTED) {
    reconnect();
  }
  //publisher
  MQTTPublisher();
  //subscriber
  client.loop();
  
  if(millis()-lastRenewal > RENEW_SUBSCRIPTION_TIME*1000){
    renewSubscription();
  }
  delay(REFRESH);
}

