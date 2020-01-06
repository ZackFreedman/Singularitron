int pins[] = {7, 8, 26, 31};

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 4; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }

//  pinMode(8, OUTPUT);
//  digitalWrite(8, HIGH);

}

void loop() {
  // put your main code here, to run repeatedly:
//  for (int i = 0; i < 4; i++) digitalWrite(pins[i], HIGH);
//  delay(1000);
//  for (int i = 0; i < 4; i++) digitalWrite(pins[i], LOW);
//  delay(1000);

  for (int i = 0; i < 4; i++) {
    digitalWrite(pins[i], HIGH);
    delay(1000);
    digitalWrite(pins[i], LOW);
  }

}
