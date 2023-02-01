void setup() {
  pinMode(33, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
}

void loop() {
  digitalWrite(36, HIGH);
  delay(1000);
  digitalWrite(35, HIGH);
  delay(1000);
  digitalWrite(34, HIGH);
  delay(1000);
  digitalWrite(33, HIGH);
  delay(1000);
  digitalWrite(33, LOW);
  delay(1000);
  digitalWrite(34, LOW);
  delay(1000);
  digitalWrite(35, LOW);
  delay(1000);
  digitalWrite(36, LOW);
  delay(1000);
}
