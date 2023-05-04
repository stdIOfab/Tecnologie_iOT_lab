//Software non ottimale dal punto di vista energetico, ma essendo necessario l'ascolto continuo dal microfono e' pressoche' inutile cercare di ottimizzare ulteriormente i consumi

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
const int RLED_PIN = A1;
const int GLED_PIN = A2;
const byte MIN_BRIGHTNESS = 0;
const byte MAX_BRIGHTNESS = 255;
byte brightness;

//PIR var and const
const int PIR_PIN = 4;
unsigned int timeout_PIR = 30; //minuti
unsigned long int last_PIR_presence = 0; //millis() del momento in cui e' stata rilevata l'ultima presenza PIR

//Microphone var and const
const unsigned int SAMPLE_FREQ = 36000; //campionamento a 36 kHz
const unsigned short N_CHANNELS = 1; //canale mono
const unsigned int SOUND_THRESHOLD = 3000; //ampiezza del valore campionato che aziona il battito
int n_sound_events = 0; //numero di eventi oltre la SOUND_THRESHOLD, si parte con 0
unsigned long int first_over_threshold; //millis() del primo valore oltre SOUND_THRESHOLD
unsigned long int last_over_threshold; //millis() dell'ultimo valore oltre SOUND_THRESHOLD
const unsigned int MAX_PRESENCE_THRESHOLD = 3200; //numero di eventi che devono verificarsi per definire il battito di mani
const unsigned int MIN_PRESENCE_THRESHOLD = 300; //numero di eventi sopra il quale viene filtrato il rumore
const unsigned int CLAP_MAX_DURATION = 100; //durata massima di un battito di mani in ms
const unsigned int CLAP_MIN_DURATION = 50; //durata minima di un battito di mani in ms
bool first_clap = false; //flag che determina l'avvenimento del primo battito
unsigned long int first_clap_end = 0; //millis() del rilevamento fine primo clap
int clap_pause = false; //flag che verifica il periodo di "silenzio" tra un clap e l'altro
const int min_clap_break = 300; //tempo minimo di pausa tra un clap e l'altro
const int max_clap_break = 600; //tempo massimo di pausa tra un clap e l'altro
int double_clap = 0; //tiene conto dei clap effettuati e tramite un operazione di resto accende e spegne il LED tra un clap e l'altro
short sampleBuffer[512]; //buffer per i dati PDM provenienti adl microfono
int samplesRead = 0; //byte letti PDM

//LCD const and var
const int ADDRESS_I2C = 0xA0; //indirizzo i2c dello schermo LCD
LiquidCrystal_PCF8574 lcd(ADDRESS_I2C);
short LCD_ready = -1; //indica se lo schermo LCD e' stato avviato correttamente
const int LCD_DELAY = 4000; //indica la frequenza di aggiornamento del display LCD
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
unsigned int REFRESH = 3000; //REFRESH dei valori del programma dal loop()
volatile bool presence = false; //indica se è presente qualcuno a nella stanza
volatile byte auto_eco = 0; //indica se è attiva la modalita' di risparmio energetica automatica, abilitata di default
volatile byte system_status = 1; //indica se i sistemi di riscaldamento e raffreddamento sono attivi

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

//attraverso i vari timeout del mic e del PIR aggiorna costantemente la variabile presence per determinare se è presente qualcuno in casa
void checkPresence(){
  if((millis() - last_PIR_presence) < (60000*timeout_PIR) && last_PIR_presence != 0){
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
  
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Monitor Seriale inizializzato...");
  
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
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), PIRpresenceDetected, RISING);
  PDM.onReceive(onPDMdata);
}

void loop() {
  if(samplesRead != 0){
    for(int i=0; i < samplesRead; i++){
      if(abs(sampleBuffer[i]) > SOUND_THRESHOLD){
        if(n_sound_events == 0){
          first_over_threshold = millis();
        }
        n_sound_events++;
        last_over_threshold = millis();
        }       
    }
    if(n_sound_events > MAX_PRESENCE_THRESHOLD && (last_over_threshold-first_over_threshold) > CLAP_MIN_DURATION && (last_over_threshold-first_over_threshold) < CLAP_MAX_DURATION){
      if(!first_clap){
        first_clap = true;
        first_clap_end = millis();
      }
      if(first_clap && clap_pause){
        Serial.println(double_clap++);
        first_clap = false;
        clap_pause = false;
      }
      n_sound_events = 0;
      //Serial.println("Reset");
    }
    if(first_clap && (millis()-first_clap_end) > min_clap_break && n_sound_events < MIN_PRESENCE_THRESHOLD){
      clap_pause = true;
    }
    if(millis()-first_clap_end > max_clap_break){
      clap_pause = false;
      first_clap = false;
    }
    if(millis()-first_over_threshold > CLAP_MAX_DURATION && !first_clap){
      //Serial.println("Timeout");
      n_sound_events = 0;
    }
  }
  if(double_clap%2 == 0){
    digitalWrite(GLED_PIN, LOW);
  }else{
    digitalWrite(GLED_PIN, HIGH);
  }
  checkCooling();
  checkHeating();
}
