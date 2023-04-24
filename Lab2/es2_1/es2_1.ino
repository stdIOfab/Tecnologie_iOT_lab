#include <Scheduler.h>

//Temp var and const
const int B = 4275;
const int R0 = 100000;
const int T0 = 25;
const float KELVIN = 273.15;
const int TEMPSENSOR_PIN = 14;

//Fan var and const
const int FAN_PIN = 5;
const int MIN_SPEED = 70;
const int MAX_SPEED = 255;
int dutyCycle = 0;

//Local
volatile float maxTemp = 30, minTemp = 25;

//funzione per la lettura della temperatura dal sensore, restituisce in float i gradi centigradi
float readTemp(){
  int rawRead;
  float temp, R;
  rawRead = analogRead(TEMPSENSOR_PIN);
    
  R = (1023.0/rawRead)-1.0;
  R = R0*R;

  temp = 1.0/((log(R/R0)/B)+(1/(KELVIN+T0)))-KELVIN;
  
  return temp;
}

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

//verifica attraverso lo Scheduler la presenza di dati in input dal terminale, li verifica e nel caso aggiorna tempMax e tempMin
void checkInput(){
  String letter;
  float check;
  if(Serial.available() > 0){
    letter = Serial.readString();
    letter.toUpperCase();
    if(letter.substring(0, 4).equals("MIN:") && isFloat(letter.substring(4))){
      check = letter.substring(4).toFloat();
      if(check < maxTemp){
        minTemp = check;
        Serial.print("Temperatura minima impostata a: ");
        Serial.println(minTemp);
      }else{
        Serial.println("Temperatura minima inserita superiore o uguale a quella massima. Riprovare.");
      }
    }else if(letter.substring(0, 4).equals("MAX:") && isFloat(letter.substring(4))){
      check = letter.substring(4).toFloat();
      if(check > minTemp){
        maxTemp = check;
        Serial.print("Temperatura massima impostata a: ");
        Serial.println(maxTemp);
      }else{
        Serial.println("Temperatura massima inserita inferiore o uguale a quella minima. Riprovare.");
      }
    }else{
      Serial.println("Formato di temperatura inserito non corretto.");
    }
}
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Monitor Seriale inizializzato...");
  Serial.println("Per aggiornarnare tempMin e tempMax digitare \"MIN:<temp>\"");
  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, dutyCycle);
  Scheduler.startLoop(checkInput);
}

void loop() {
  Serial.print("Temperatura misurata: ");
  Serial.println(readTemp());
  if(readTemp() >= minTemp && readTemp() <= maxTemp){
    dutyCycle = map(readTemp(), minTemp, maxTemp, MIN_SPEED, MAX_SPEED);
    analogWrite(FAN_PIN, dutyCycle);
  }else if(readTemp() > maxTemp){
    analogWrite(FAN_PIN, MAX_SPEED);
  }else{
    analogWrite(FAN_PIN, 0);
  }
  delay(2000);
}
