#define chargeStatusPin 33
#define vibes 6

volatile bool unhandledChange;
volatile bool lastChangeType;
volatile unsigned long lastUpTimestamp;
volatile unsigned long lastDownTimestamp;

void setup() {
  // put your setup code here, to run once:
  pinMode(vibes, OUTPUT);
  digitalWrite(vibes, LOW);
  pinMode(chargeStatusPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(chargeStatusPin), onBatteryEvent, CHANGE);
  Serial.begin(9600);
}

void loop() {
  Serial.println((micros() - lastDownTimestamp) + (micros() - lastUpTimestamp));
}

//void onBatteryEvent() {
//  unhandledChange = true;
//  lastDownTimestamp = micros();
//}

//void loop() {
//  // put your main code here, to run repeatedly:
//  if (unhandledChange) {
//    Serial.println(lastDownTimestamp - lastUpTimestamp);
//    unhandledChange = false;
//  }
//
//  //Serial.println(micros() - lastChangeTimestamp);
//}
//
void onBatteryEvent() {
  lastChangeType = !digitalReadFast(chargeStatusPin);
  if (lastChangeType)
    lastUpTimestamp = micros();
  else {
    lastDownTimestamp = micros();
    unhandledChange = true;
  }
}

