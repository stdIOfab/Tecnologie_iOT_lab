#include <math.h>

const int B = 4275;
const int R0 = 100000;
const int T0 = 25;
const float KELVIN = 273.15;
const int TEMPSENSOR_PIN = 14;
int rawRead;
float R, temp;

void setup(){
    Serial.begin(9600);
    while(!Serial);
    Serial.println("Monitor Seriale inizializzato...");
}

void loop()
{
    rawRead = analogRead(TEMPSENSOR_PIN);
    
    R = (1023.0/rawRead)-1.0;
    R = R0*R;

    temp = 1.0/((log(R/R0)/B)+(1/(KELVIN+T0)))-KELVIN;

    Serial.print("Temperatura rilevata: ");
    Serial.println(temp);

    delay(1000);
}
