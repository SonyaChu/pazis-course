#define PHOTO_PIN 34   

hw_timer_t *timer1 = NULL;  
hw_timer_t *timer2 = NULL;  
volatile int photoValue1 = 0;  
volatile int photoValue2 = 0;  

void IRAM_ATTR onTimer1() {
  photoValue1 = analogRead(PHOTO_PIN); 
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" мс] Timer1 -> ");
  Serial.println(photoValue1);
}

void IRAM_ATTR onTimer2() {
  photoValue2 = analogRead(PHOTO_PIN); 
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" мс] Timer2 -> ");
  Serial.println(photoValue2);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  /
  pinMode(PHOTO_PIN, INPUT);
  Serial.println("=== Запуск таймерів ===");
  timer1 = timerBegin(1'000'000);             
  timerAttachInterrupt(timer1, &onTimer1);
  timerAlarm(timer1, 500'000, true, 0);       
  timer2 = timerBegin(1'000'000);
  timerAttachInterrupt(timer2, &onTimer2);
  timerAlarm(timer2, 2'000'000, true, 0);
  Serial.println("Таймери запущено!");
}

void loop() {
  
}


