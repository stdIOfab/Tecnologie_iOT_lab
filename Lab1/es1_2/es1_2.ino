#include <MBED_RPi_Pico_TimerInterrupt.h>
const int RLED_PIN = 2;
const int BLED_PIN = 3;
const long R_HALF_PERIOD = 3000L;
const long B_HALF_PERIOD = 4000L;
unsigned long delay_time;
int redLedState = LOW;
int blueLedState = LOW;
MBED_RPI_PICO_Timer ITimer1(1);

void checkLed(){
  if(Serial.available()>0){
    int letter=Serial.read();
    if (letter == 'r' || letter == 'R'){
      Serial.print("LED rosso: ");
      Serial.println(redLedState);
    }
    else if (letter == 'b' || letter == 'B'){
      Serial.print("LED blu: ");
      Serial.println(blueLedState);
    }
    else {
      Serial.println("Carattere non riconosciuto");
    }
  }
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
}

void loop(){
  checkLed();
  digitalWrite(RLED_PIN, redLedState);
  redLedState = !redLedState;
  delay_time = millis();
  while(millis() <= delay_time+R_HALF_PERIOD);
}