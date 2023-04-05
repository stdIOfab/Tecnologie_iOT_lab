//#include <MBED_RPi_Pico_TimerInterrupt.h>
#include <MBED_RPi_Pico_TimerInterrupt.h>
const int Rled_pin = 2; 
const int Bled_pin = 3; 

const long r_half_period = 100L;
const long g_half_period = 250L;

int redlstate = LOW;
int greenlstate = LOW;

MBED_RPI_PICO_Timer ITimer1(1); 

void setup() {
  pinMode(rled_pin, OUTPUT);
  pinMode(gled_pin, OUTPUT);
  ITimer1.setInterval(g_half_period*1000, blinkGreen) ;
}

void blinkGreen(uint alarm){
  TIMER_ISR_START(alarm);
  digitalWrite(gled_pin, greenlstate);
  greenlstate = !greenlstate; 
  TIMER_ISR_END(alarm); 
} 

void loop() {
   digitalWrite(rled_pin, redlstate);
   redlstate = !redlstate; 
   delay(r_half_period);
}


  
