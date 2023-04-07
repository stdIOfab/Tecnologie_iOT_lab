const int PIR_PIN = 4;
const int BLED_PIN = 3;
int blueLedState = LOW;
volatile int tot_count = 0;

void checkPresence(){
  if(blueLedState != HIGH){
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
  // put your main code here, to run repeatedly:
  delay(30000);
  Serial.print("Eventi registrati: ");
  Serial.println(tot_count);
}
