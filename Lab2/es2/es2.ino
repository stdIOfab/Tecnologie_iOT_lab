#include <Scheduler.h>
#include <PDM.h>

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
byte dutyCycle = 0;
volatile float coolingMaxTemp = 30, coolingMinTemp = 25;
volatile float heatingMaxTemp = 24, heatingMinTemp = 18;

//LED var and const 
const int RLED_PIN = A1;
const byte MIN_BRIGHTNESS = 0;
const byte MAX_BRIGHTNESS = 255;
byte brightness;

//PIR var and const
const int PIR_PIN = 4;
unsigned int timeout_PIR = 30; //riferito a minuti
unsigned long int last_presence = 0;
volatile bool presence = false;

//Microphone var and const
const unsigned int SAMPLE_FREQ = 16000;
const unsigned short N_CHANNELS = 1;
const unsigned int SOUND_THRESHOLD = 500;
volatile int n_sound_events = 0;
unsigned int timeout_MIC = 10; //riferito a minuti
const unsigned int PRESENCE_THRESHOLD = 20;
short sampleBuffer[256];
volatile int samplesRead = 0;
const unsigned int NEW_TRY_PDM = 1500;//tempo in ms

//Program var

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
    if(letter.substring(0, 5).equals("CMIN:") && isFloat(letter.substring(5))){
      check = letter.substring(5).toFloat();
      if(check < coolingMaxTemp && check > heatingMaxTemp){
        coolingMinTemp = check;
        Serial.print("Temperatura di raffrescamento minima impostata a: ");
        Serial.println(coolingMinTemp);
      }else{
        Serial.println("Intervallo di temperatura non valido. Riprovare.");
      }
    }else if(letter.substring(0, 5).equals("CMAX:") && isFloat(letter.substring(5))){
      check = letter.substring(5).toFloat();
      if(check > coolingMinTemp){
        coolingMaxTemp = check;
        Serial.print("Temperatura di raffrescamento massima impostata a: ");
        Serial.println(coolingMaxTemp);
      }else{
        Serial.println("Intervallo di temperatura non valido. Riprovare.");
      }
    }else if(letter.substring(0, 5).equals("HMIN:") && isFloat(letter.substring(5))){
      check = letter.substring(5).toFloat();
      if(check < heatingMaxTemp){
        heatingMinTemp = check;
        Serial.print("Temperatura di riscaldamento minima impostata a: ");
        Serial.println(heatingMinTemp);
      }else{
        Serial.println("Intervallo di temperatura non valido. Riprovare.");
      }
    }else if(letter.substring(0, 5).equals("HMAX:") && isFloat(letter.substring(5))){
      check = letter.substring(5).toFloat();
      if(check > heatingMinTemp && check < coolingMinTemp){
        heatingMaxTemp = check;
        Serial.print("Temperatura di riscaldamento massima impostata a: ");
        Serial.println(heatingMaxTemp);
      }else{
        Serial.println("Intervallo di temperatura non valido. Riprovare.");
      }
    }else if(letter.substring(0, 12).equals("TIMEOUT_PIR:") && isFloat(letter.substring(12))){
      timeout_PIR = abs(letter.substring(12).toInt());
      Serial.print("Timeout rilevamento persone impostato a: ");
      Serial.print(timeout_PIR);
      Serial.println(" minuti.");
    }
    else{
      Serial.println("Inserimento non corretto.");
    }
  }
}

void presenceDetected(){
  last_presence = millis();
}

void checkPresence(){
  if(((millis() - last_presence) < (60000*timeout_PIR) && last_presence != 0) || ((millis() - last_presence) < (60000*timeout_MIC) && last_presence != 0)){
    presence = true;
  }else{
    presence = false;
  }
  yield();
}

void checkCooling(){
  if(readTemp() >= coolingMinTemp && readTemp() <= coolingMaxTemp){
    dutyCycle = map(readTemp(), coolingMinTemp, coolingMaxTemp, MIN_SPEED, MAX_SPEED);
  }else if(readTemp() > coolingMaxTemp){
    dutyCycle = MAX_SPEED;
  }else{
    dutyCycle = 0;
  }
  analogWrite(FAN_PIN, dutyCycle);
}

void checkHeating(){
  if(readTemp() >= heatingMinTemp && readTemp() <= heatingMaxTemp){
    brightness = map(readTemp(), heatingMinTemp, heatingMaxTemp, MAX_BRIGHTNESS, MIN_BRIGHTNESS);
  }else if(readTemp() > heatingMaxTemp){
    brightness = MIN_BRIGHTNESS;
  }else{
    brightness = MAX_BRIGHTNESS;
  }
  analogWrite(RLED_PIN, brightness);
}

void onPDMdata(){
  int bytesAvailable = PDM.available();
  samplesRead = PDM.read(sampleBuffer, bytesAvailable) / 2;
}

void micMustDo(){};

void printMenu(){};

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Monitor Seriale inizializzato...");
  if(!PDM.begin(1, 16000)) {
    Serial.println("Impossibile avviare i processi per l'ascolto dal microfono integrato, in attesa di un nuovo tentativo...");
    while (1);
  }
  Serial.println("Processi per l'ascolto dal microfono integrato avviati.");
  if(!PDM.begin(N_CHANNELS, SAMPLE_FREQ)){
    Serial.println();
  }
  PDM.setBufferSize(256);
  printMenu();
  Serial.println("Per aggiornarnare tempMin e tempMax digitare \"MIN:<temp>\"");
  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, dutyCycle);
  Scheduler.startLoop(checkInput);
  Scheduler.startLoop(checkPresence);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), presenceDetected, RISING);
  PDM.onReceive(onPDMdata);
}

void loop() {
  /*Serial.print("Temperatura misurata: ");
  Serial.println(readTemp());
  checkCooling();
  checkHeating();
  */
  if(samplesRead!=0){
    for(int i=0; i < samplesRead; i++){
      if(N_CHANNELS == 2){
        Serial.print("L:");
        Serial.print(sampleBuffer[i]);
        i++;
    }
      //Serial.print(" R:");
      if(abs(sampleBuffer[i]) > SOUND_THRESHOLD){
        Serial.println("1");
        n_sound_events++;
        if(n_sound_events >= PRESENCE_THRESHOLD){
          presenceDetected()
        }       
      }else{
        Serial.println("0");
      }
    }
    samplesRead = 0;
  }
  delay(10);
}
