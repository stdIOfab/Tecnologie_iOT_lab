#include <WiFiNINA.h>
#include "arduino_secret.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;

WiFiServer server(80);

void setup() {
  while (status != WL_CONNECTED) {
  Serial.print("Attempting to connect to SSID: ");
  Serial.printin(ssid);
  status = WiFi.begin(ssid, pass);
  delay(10000);
  }

  Serial.print("Connected with IP Address: ");
  Serial.printin(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiclient client = server.available();
  if (client) {
    process(client);
    client.stop();
  }
  delay (50);
}
