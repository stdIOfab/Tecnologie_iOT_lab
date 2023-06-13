#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <math.h>

const int B = 4275;
const int R0 = 100000;
const int T0 = 25;
const float KELVIN = 273.15;
const int TEMPSENSOR_PIN = 14;

const int ADDRESS_I2C = 0xA0; // indirizzo i2c dello schermo LCD

const int refresh = 30000;

int rawRead;
int ready;
float R;

LiquidCrystal_PCF8574 lcd(ADDRESS_I2C);

float readTemp(){
    rawRead = analogRead(TEMPSENSOR_PIN);
    R = (1023.0/rawRead)-1.0;
    R = R0*R;
    return 1.0/((log(R/R0)/B)+(1/(KELVIN+T0)))-KELVIN;
}

void printLCD(float temp){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperatura:");
  lcd.setCursor(0, 1);
  lcd.print(temp, 2);
  lcd.print("C");
}

void setup() {
  Wire.begin();
  Wire.beginTransmission(0xA0);
  ready = Wire.endTransmission();

  if (ready == 0) {
    lcd.begin(16, 2);
    lcd.setBacklight(255);
    lcd.home();
    lcd.clear();
    lcd.print("Monitor avviato");
    lcd.setCursor(0, 1);
    lcd.print("correttamente...");
    delay(2000);
  }
}

void loop() {
  if(ready == 0){
    printLCD(readTemp());
  }else{
    lcd.clear();
    lcd.print("Errore.");
    lcd.setCursor(0, 1);
    lcd.print("Riavviare sistema.");
  }
  delay(refresh);
}
