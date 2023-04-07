#include <MBED_RPi_Pico_TimerInterrupt.h>
const int RLED_PIN = 2;
const int BLED_PIN = 3;
const long R_HALF_PERIOD = 3000L;
const long B_HALF_PERIOD = 4000L;
unsigned long delay_time;
int redLedState = LOW;
int blueLedState = LOW;
boolean newData;
String letter;
int incBytes = 0;
#include <Scheduler.h>
MBED_RPI_PICO_Timer ITimer1(1);


void checkLedWrapper(){
  if(Serial.available() == 1){
  letter = Serial.readString();
  printStatus();
  }else if(Serial.available() > 1){
    Serial.readString(); //svuota il buffer 
    Serial.println("Troppi input inseriti, riprova.");
  }
}

void printStatus(){
  if(letter[0] == 'r' || letter[0] == 'R'){
    Serial.print("LED rosso: ");
    if(redLedState == LOW){
      Serial.println("HIGH");
    }else{
      Serial.println("LOW");
    }
  }
  else if(letter[0] == 'b' || letter[0] == 'B'){
    Serial.print("LED blu: ");
    if(blueLedState == LOW){
      Serial.println("HIGH");
    }else{
      Serial.println("LOW");
    }
  }
  else{
    Serial.println("Carattere non riconosciuto");
  }
  yield();
}

void blinkBlue(uint alarm_num){
  TIMER_ISR_START(alarm_num);
  digitalWrite(BLED_PIN, blueLedState);
  blueLedState = !blueLedState;
  TIMER_ISR_END(alarm_num);
}

void setup(){
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Benvenuto!");
  pinMode(RLED_PIN, OUTPUT);
  pinMode(BLED_PIN, OUTPUT);
  ITimer1.setInterval(B_HALF_PERIOD * 1000, blinkBlue);
  Scheduler.startLoop(checkLedWrapper);
}

void loop(){
  digitalWrite(RLED_PIN, redLedState);
  redLedState = !redLedState;
  delay(R_HALF_PERIOD);
}