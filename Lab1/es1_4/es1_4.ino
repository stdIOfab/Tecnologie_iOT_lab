const int FAN_PIN = 5;
const int increment = 10;
int dutyCycle = 0;
char input;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Monitor Seriale inizializzato...");
  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, dutyCycle);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    input = Serial.read();
    if(input == '+'){
      dutyCycle += increment;
    }else if(input == '-'){
      dutyCycle -= increment;
    }else{
      Serial.println("Errore, inseriti caratteri non validi");
      Serial.readString(); //svuota il buffer
    }
    if(dutyCycle < 0){
      Serial.println("Velocità minima gia' raggiunta.");
      dutyCycle += increment;
      Serial.readString(); //svuota il buffer
    }else if(dutyCycle > 255){
      Serial.println("Velocità massima gia' raggiunta.");
      dutyCycle -= increment;
      Serial.readString(); //svuota il buffer
    }else{
      analogWrite(FAN_PIN, dutyCycle);
    }
  }
}
