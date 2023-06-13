#include <MBED_RPi_Pico_TimerInterrupt.h>
const int RLED_PIN = 2;  //LED rosso collegato al pin 3
const int BLED_PIN = 3;  //LED blu collegato al pin 2

const long R_HALF_PERIOD = 200L;
const long B_HALF_PERIOD = 300L;

int redLedState = LOW;
int blueLedState = LOW;

MBED_RPI_PICO_Timer ITimer1(1);

void blinkBlue(uint alarm_num){
  TIMER_ISR_START(alarm_num);
  digitalWrite(BLED_PIN, blueLedState);
  blueLedState = !blueLedState;
  TIMER_ISR_END(alarm_num);
}

void setup() {
  pinMode(RLED_PIN, OUTPUT);
  pinMode(BLED_PIN, OUTPUT);
  ITimer1.setInterval(B_HALF_PERIOD*1000, blinkBlue);
}

void loop() {
   digitalWrite(RLED_PIN, redLedState);
   redLedState = !redLedState; 
   delay(R_HALF_PERIOD);
}


  
