#define CHARGE_STATUS_PIN 33

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(CHARGE_STATUS_PIN, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(digitalRead(CHARGE_STATUS_PIN));

  delay(100);
}
