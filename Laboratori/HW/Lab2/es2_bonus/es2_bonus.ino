//Software non ottimale dal punto di vista energetico, in quanto l'arduino non viene mai messo in stato di bassa potenza, tuttavia essendo necessario l'ascolto continuo dal microfono e' pressoche' inutile cercare di ottimizzare ulteriormente i consumi

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
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
const int MIN_SPEED = 100; //valore maggiore di 0 in quanto il motore non riesce a mettersi in rotazione sotto questa soglia
const int MAX_SPEED = 255;//massimo valore PWM
byte dutyCycle = 0;

//LED var and const 
const int RLED_PIN = 2;
const int BLED_PIN = 3;
byte bled_status = LOW;
const byte MIN_BRIGHTNESS = 0;
const byte MAX_BRIGHTNESS = 255;
byte brightness = 0;

//PIR var and const
const int PIR_PIN = 4;
unsigned int timeout_PIR = 30; //minuti
unsigned long int last_PIR_presence = 0; //millis() del momento in cui e' stata rilevata l'ultima presenza PIR

//Microphone var and const
const unsigned int SAMPLE_FREQ = 16000; //campionamento a 16 kHz
const unsigned short N_CHANNELS = 1; //canale mono
short sampleBuffer[256]; //buffer per i dati PDM provenienti adl microfono
unsigned int sound_threshold = 2000; //ampiezza del valore campionato che aziona la presenza
volatile int samplesRead = 0; //byte letti PDM
volatile unsigned long int last_mic_presence = 0; //millis() del momento in cui e' stata rilevata l'ultima presenza mic
unsigned int timeout_mic = 10; //riferito a minuti
/*
const unsigned int NOISE_TIMEOUT = 25; //secondi
const unsigned int SOUND_REFRESH = 500; //ms
volatile int n_sound_events = 0; //numero di eventi che appurano la presenza, si parte con 0
unsigned long int last_over_threshold;
const unsigned int PRESENCE_THRESHOLD = 10; //soglia oltre la quale viene appurata la presenza
*/
const int CLAP_DURATION = 200;
const int CLAP_MAX_BREAK = 800;
const int THIRD_CLAP_FILTER = 1000;
int lastSoundValue;
int soundValue;
long lastNoiseTime = 0;
long currentNoiseTime = 0;
long lastLightChange = 0;

//LCD const and var
const int ADDRESS_I2C = 0xA0; //indirizzo i2c dello schermo LCD
LiquidCrystal_PCF8574 lcd(ADDRESS_I2C);
short LCD_ready = -1; //indica se lo schermo LCD e' stato avviato correttamente
const int LCD_DELAY = 4000; //indica la frequenza di aggiornamento del display LCD

//caratteri personalizzati utilizzati nel display LCD
byte ecoChar[8] = {
	0b00100,
	0b01010,
	0b10001,
	0b10101,
	0b10101,
	0b01110,
	0b00100,
	0b00100
};

byte presenceChar[8] = {
	0b01110,
	0b01110,
	0b00100,
	0b11111,
	0b10101,
	0b00100,
	0b01010,
	0b11011
};

//Program var
volatile float coolingMaxTemp = 30, coolingMinTemp = 25; //setpoint AC
volatile float heatingMaxTemp = 24, heatingMinTemp = 18;  //setpoint HT
volatile float heating_cooling_delta = 5; //se non vengono rilevate persone abbassa/alza del valore i setpoint mettendosi in modalità risparmio energetico
unsigned int REFRESH = 1500; //REFRESH dei valori del programma dal loop()
volatile bool presence = false; //indica se è presente qualcuno a nella stanza
volatile byte auto_eco = 0; //indica se è attiva la modalita' di risparmio energetica automatica, abilitata di default
volatile byte system_status = 1; //indica se i sistemi di riscaldamento e raffreddamento sono attivi

//funzione per la lettura della temperatura dal sensore, restituisce in float i gradi centigradi
float readTemp(){
  int rawRead;
  float R;
  rawRead = analogRead(TEMPSENSOR_PIN);
    
  R = (1023.0/rawRead)-1.0;
  R = R0*R;

  return 1.0/((log(R/R0)/B)+(1/(KELVIN+T0)))-KELVIN;
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

//la funzione di libreria map crea una mappatura su interi, operando su float va creata una nuova funzione analoga
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (((x - in_min) * (out_max - out_min) / (in_max - in_min)) + out_min);
}

//verifica attraverso lo Scheduler la presenza di dati in input dal terminale, li verifica e nel caso aggiorna le variabili globali
void checkInput(){
  String letter;
  float check;
  if(Serial.available() > 0){
    letter = Serial.readString();
    letter.toUpperCase();
		if(letter.charAt(letter.length()-1)=='\n'){
			letter = letter.substring(0, letter.length()-1);
  		}
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
      check = abs(letter.substring(12).toInt());
      if(check > 0){
        timeout_PIR = check;
        Serial.print("Timeout rilevamento persone tramite sensorre PIR impostato a: ");
        Serial.print(timeout_PIR);
        Serial.println(" minuti.");
      }else{
        Serial.println("Valore inserito troppo basso. Riprova.");
      }
    }
    else if(letter.substring(0, 16).equals("SOUND_THRESHOLD:") && isFloat(letter.substring(16))){
      check = letter.substring(16).toInt();
      if(check > 0){
        sound_threshold = check;
        Serial.print("Soglia rilevamento suoni ambientali impostata a: ");
        Serial.println(sound_threshold);
      }else{
        Serial.println("Valore inserito troppo basso, riprova.");
      }
    }/*
    else if(letter.substring(0, 12).equals("TIMEOUT_MIC:") && isFloat(letter.substring(12))){
      check = letter.substring(12).toInt();
      if(check >= 2*NOISE_TIMEOUT/60){
        timeout_mic = check;
        Serial.print("Timeout rilevamento persone tramite microfono integrato impostato a: ");
        Serial.print(timeout_mic);
        Serial.println(" minuti.");
      }else{
        Serial.println("Valore inserito troppo basso. Riprova.");
      }
    }*/
    else if(letter.substring(0, 9).equals("HC_DELTA:") && isFloat(letter.substring(9))){
      heating_cooling_delta = abs(letter.substring(9).toFloat());
      Serial.print("Delta temperatura risparmio energetico in modalità assenza impostata a: ");
      Serial.println(heating_cooling_delta);
    }
    else if(letter.substring(0, 9).equals("AUTO_ECO:") && isFloat(letter.substring(9))){
      check = letter.substring(9).toInt();
      if(check == 0 || check == 1){
        auto_eco = check;
        Serial.print("Modalita' risparmio energetico: ");
        Serial.println(auto_eco);
      }else{
        Serial.println("Modalita' non riconosciuta");
      }
    }
    else if(letter.substring(0, 11).equals("SYS_STATUS:") && isFloat(letter.substring(11))){
      check = letter.substring(11).toInt();
      if(check == 1){
        system_status = check;
        Serial.println("Sistema attivato.");
      }
      else if(check == 0){
        system_status = check;
        Serial.println("Sistema disattivato.");
      }else{
        Serial.println("Modalita' non riconosciuta");
      }
    }
    else{
      Serial.println("Inserimento non corretto.");
    }
  }
  yield();
}

//aggiorna la variabile inserendo quando è stato rilevato l'evento
void PIRpresenceDetected(){
  last_PIR_presence = millis();
}

/*
void MICpresenceDetected(){
  last_mic_presence = millis();
}
*/

//attraverso i vari timeout del mic e del PIR aggiorna costantemente la variabile presence per determinare se è presente qualcuno in casa
void checkPresence(){
  if((((millis() - last_PIR_presence) < (60000*timeout_PIR) && last_PIR_presence != 0) || ((millis() - last_mic_presence) < (60000*timeout_mic) && last_mic_presence != 0))){
    presence = true;
  }else{
    presence = false;
  }
  yield();
}

//attiva o disattiva la ventola AC
void checkCooling(){
  int real_delta = 0;
  if(!presence && auto_eco == 1){
    real_delta = heating_cooling_delta;
  }
  if(readTemp() >= coolingMinTemp+real_delta && readTemp() <= coolingMaxTemp+real_delta && system_status == 1){
    dutyCycle = mapFloat(readTemp(), coolingMinTemp+real_delta, coolingMaxTemp+real_delta, MIN_SPEED, MAX_SPEED);
  }else if(readTemp() > coolingMaxTemp+real_delta && system_status == 1){
    dutyCycle = MAX_SPEED;
  }else{
    dutyCycle = 0;
  }
  analogWrite(FAN_PIN, dutyCycle);
}

//attiva o disattiva il sistema HT
void checkHeating(){
  int real_delta = 0;
  if(!presence && auto_eco == 1){
    real_delta = heating_cooling_delta;
  }
  if(readTemp() >= heatingMinTemp-real_delta && readTemp() <= heatingMaxTemp-real_delta && system_status == 1){
    brightness = mapFloat(readTemp(), heatingMinTemp-real_delta, heatingMaxTemp-real_delta, MAX_BRIGHTNESS, MIN_BRIGHTNESS);
  }else if(readTemp() < heatingMinTemp-real_delta && system_status == 1){
    brightness = MAX_BRIGHTNESS;
  }else{
    brightness = MIN_BRIGHTNESS;
  }
  analogWrite(RLED_PIN, brightness);
}

//ISR per i dati provenienti dal mic
void onPDMdata(){
  int bytesAvailable = PDM.available();
  samplesRead = PDM.read(sampleBuffer, bytesAvailable) / 2;
}

/*
//controlla i dati provenienti dal microfono per verificare se vi è una presenza all'interno della casa
void checkSound(){
  if(samplesRead != 0){
    for(int i=0; i < samplesRead; i++){
      if(abs(sampleBuffer[i]) > sound_threshold){
        last_over_threshold = millis();
        n_sound_events++;
        break;
        }       
    }
    if(millis() - last_over_threshold < 1000*NOISE_TIMEOUT){
      if(n_sound_events > PRESENCE_THRESHOLD){
        MICpresenceDetected();
      }
    }else{
      n_sound_events = 0;
    }
    samplesRead = 0;
  }
  yield();
  delay(SOUND_REFRESH);
}
*/

void doubleClapDetector(){
  soundValue = 0; //se il ciclo successivo non cambia la variabile si suppone uguale a 0
  if(samplesRead != 0){
    for(int i=0; i < samplesRead; i++){
      if(abs(sampleBuffer[i]) > sound_threshold){
        soundValue = 1;
        break; //interrompe il ciclo sull'array in quanto un valore oltre la soglia è già stato trovato
      }
    }
  }
  currentNoiseTime = millis();
  if(soundValue == 1){
    if(
      (currentNoiseTime > lastNoiseTime + CLAP_DURATION) && // Verifica che il rumore sia piu' lungo di CLAP_DURATION ms
      (lastSoundValue == 0) &&  // verifica il silenzio tra un clap e il successivo oltre
      (currentNoiseTime < lastNoiseTime + CLAP_MAX_BREAK) && // Verifica che il clap successivo sia entro CLAP_MAX_DURATION ms
      (currentNoiseTime > lastLightChange + THIRD_CLAP_FILTER) // Filtra di THIRD_CLAP_FILTER ms affinchè non venga riconosciuto un terzo clap
    ){
      bled_status = !bled_status;   // Modifica lo stato del LED
      digitalWrite(BLED_PIN, bled_status);
      lastLightChange = currentNoiseTime;
    }
    lastNoiseTime = currentNoiseTime;
  }
  lastSoundValue = soundValue;
  yield();
}

//stampa del menu iniziale sul seriale
void printMenu(){
  char buffer[500];
  Serial.println("Guida ai comandi del Controller Smart Home:");
  sprintf(buffer, "Il controller gestisce le periferiche di AC e HT della casa. E' presente una lista di comandi \n attraverso i quali e' possibile modificare il comportamento del sistema. \n Ricorda che i setpoint sono soggetti ad un delta di risparmio energetico nel caso in cui \n questa modalita' sia attivata. In tal caso i display visualizzeranno il valore attuale \n tenendo conto del delta, tuttavia il setpoint originale rimarra' invariato.");
  Serial.println(buffer);
  Serial.println("  Parametri modificabili:");
  Serial.println("    - CMIN:<+/-VALUE>: modifica la temperatura minima di raffreddamento.");
  Serial.println("    - CMAX:<+/-VALUE>: modifica la temperatura massima di raffreddamento.");
  Serial.println("    - HMAX:<+/-VALUE>: modifica la temperatura massima di riscaldamento.");
  Serial.println("    - HMIN:<+/-VALUE>: modifica la temperatura minima di riscaldamento.");
  Serial.println("    - TIMEOUT_PIR:<+/-VALUE>: modifica il periodo dopo il quale, senza ulteriori movimenti, la casa viene considerata vuota.");
  Serial.println("    - TIMEOUT_MIC:<+/-VALUE>: modifica il periodo dopo il quale, senza ulteriori suoni, la casa viene considerata vuota.");
  Serial.println("    - SOUND_THRESHOLD:<+/-VALUE>: modifica la soglia di rumore per la quale il sistema rileva la presenza.");
  Serial.println("    - AUTO_ECO:<0/1>: disattiva o attiva il risparmio energetico automatico, disabilitata di default");
  Serial.println("    - SYS_STATUS:<0/1>: disattiva o attiva l'intero sistema, abilitato di default");
}

//stampa sul display LCD dei valori di temperatura, intensita sistemi AC e HT, e setpoint
void printLCD(){
  int real_delta = 0;
  if(!presence && auto_eco == 1){
    real_delta = heating_cooling_delta;
  }
  char charBuffer[16];
  lcd.clear();
  lcd.setCursor(0, 0);
  sprintf(charBuffer, "T:%.1f\C \02:%d \01:%d", readTemp(), presence, auto_eco);
  lcd.print(charBuffer);
  lcd.setCursor(0, 1);
  sprintf(charBuffer, "AC:%.0f\%% HT:%.0f\%%", (dutyCycle < MIN_SPEED)? 0 : mapFloat(dutyCycle, MIN_SPEED, MAX_SPEED, 0, 100), (brightness < MIN_BRIGHTNESS)? 0 : mapFloat(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 0, 100));
  lcd.print(charBuffer);
  delay(LCD_DELAY);
  lcd.clear();
  lcd.setCursor(0, 0);
  sprintf(charBuffer, "AC m:%.1f M:%.1f", coolingMinTemp+real_delta, coolingMaxTemp+real_delta);
  lcd.print(charBuffer);
  lcd.setCursor(0, 1);
  sprintf(charBuffer, "HT m:%.1f M:%.1f", heatingMinTemp-real_delta, heatingMaxTemp-real_delta);
  lcd.print(charBuffer);
  delay(LCD_DELAY);
  yield();
  }

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, dutyCycle);

  pinMode(BLED_PIN, OUTPUT);
  digitalWrite(BLED_PIN, bled_status);

  pinMode(RLED_PIN, OUTPUT);
  analogWrite(RLED_PIN, brightness);
  
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Monitor Seriale inizializzato...");
  
  PDM.setBufferSize(256);
  if(!PDM.begin(1, 16000)) {
    Serial.println("Impossibile avviare i processi per l'ascolto dal microfono integrato, in attesa di un nuovo tentativo...");
    while (1);
  }
  Serial.println("Processi per l'ascolto dal microfono integrato avviati.");

  printMenu();

  while(LCD_ready!=0){
  Wire.begin();
  Wire.beginTransmission(0xA0);
  LCD_ready = Wire.endTransmission();
  }
  Serial.println("Monitor LCD avviato correttamente.");
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("Monitor avviato");
  lcd.setCursor(0, 1);
  lcd.print("correttamente...");
  lcd.createChar(1, ecoChar);
  lcd.createChar(2, presenceChar);
  delay(2000);
  
  Scheduler.startLoop(checkInput);
  Scheduler.startLoop(checkPresence);
  Scheduler.startLoop(printLCD);
  //Scheduler.startLoop(checkSound);
  Scheduler.startLoop(doubleClapDetector);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), PIRpresenceDetected, RISING);
  PDM.onReceive(onPDMdata);
}

void loop() {
  checkCooling();
  checkHeating();
  delay(REFRESH);
}
