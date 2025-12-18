int LDR_Value = 0;     
int sensorPin = 13;    
int ledPin = 25;       

void setup() {
  Serial.begin(115200);
  pinMode (sensorPin, INPUT);
  pinMode (ledPin, OUTPUT);
}

void loop() {
  LDR_Value = analogRead(sensorPin);
  Serial.print("Значення LDR: ");
  Serial.println(LDR_Value);
  int dacValue = map(LDR_Value, 0, 4095, 0, 255);
  dacWrite(ledPin, dacValue);
  delay(500);
}
