const int PIR_PIN = 4;
const int BLED_PIN = 3;
int blueLedState = LOW, refresh = 30000;
volatile int tot_count = 0;

void checkPresence(){
  if(blueLedState != HIGH){ //cambia solo sui fronti di salita
    tot_count++;
  }
  digitalWrite(BLED_PIN, !blueLedState);
  blueLedState = !blueLedState;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Serial monitor inizializzato...");
  pinMode(PIR_PIN, INPUT);
  pinMode(BLED_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), checkPresence, CHANGE);
}

void loop() {
  delay(refresh);
  Serial.print("Eventi registrati: ");
  Serial.println(tot_count);
}
