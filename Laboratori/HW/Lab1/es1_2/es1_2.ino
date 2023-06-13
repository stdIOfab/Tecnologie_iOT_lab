#include <MBED_RPi_Pico_TimerInterrupt.h>
MBED_RPI_PICO_Timer ITimer0(0);
MBED_RPI_PICO_Timer ITimer1(1);
const int RLED_PIN = 2;
const int BLED_PIN = 3;
const long R_HALF_PERIOD = 3000L;
const long B_HALF_PERIOD = 4000L;
volatile byte redLedState = HIGH;
volatile byte blueLedState = HIGH;
String letter;


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
}

void blinkBlue(uint alarm_num){
  TIMER_ISR_START(alarm_num);
  digitalWrite(BLED_PIN, blueLedState);
  blueLedState = !blueLedState;
  TIMER_ISR_END(alarm_num);
}

void blinkRed(uint alarm_num){
  TIMER_ISR_START(alarm_num);
  digitalWrite(RLED_PIN, redLedState);
  redLedState = !redLedState;
  TIMER_ISR_END(alarm_num);
}

void setup(){
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Benvenuto!");
  pinMode(RLED_PIN, OUTPUT);
  pinMode(BLED_PIN, OUTPUT);
  ITimer0.setInterval(B_HALF_PERIOD * 1000, blinkBlue);
  ITimer1.setInterval(R_HALF_PERIOD * 1000, blinkRed);
}

void loop(){
  checkLedWrapper();
}