#include <math.h>

const int B = 4275;
const int R0 = 100000;
const int T0 = 25;
const float KELVIN = 273.15;
const int TEMPSENSOR_PIN = 14;
int rawRead, refresh = 1000;
float R;

void setup(){
    Serial.begin(9600);
    while(!Serial);
    Serial.println("Monitor Seriale inizializzato...");
}

float tempCheck(){
  rawRead = analogRead(TEMPSENSOR_PIN);
  R = (1023.0/rawRead)-1.0;
  R = R0*R;
  return 1.0/((log(R/R0)/B)+(1/(KELVIN+T0)))-KELVIN;
}

void loop()
{
    Serial.print("Temperatura rilevata: ");
    Serial.println(tempCheck());
    delay(refresh);
}
