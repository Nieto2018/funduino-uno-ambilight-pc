void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT); // conecta el cable verde del controlador aquí
}

void loop() {
  int state = digitalRead(2);
  Serial.println(state);
  delay(1);
}
