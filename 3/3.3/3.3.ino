#include <Wire.h>
#include <Adafruit_BMP280.h>

#define BUTTON_PIN  15  
#define LED_BMP     32  
#define LED_BTN     33  

Adafruit_BMP280 bmp;
const unsigned long bmpInterval = 2000; 
unsigned long lastBMPRead = 0;

volatile unsigned long pressStart = 0;
volatile unsigned long pressDuration = 0;
volatile bool buttonPressed = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceTime = 50; 

void IRAM_ATTR handleButton() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime < debounceTime) return; 
  lastInterruptTime = currentTime;

  if (digitalRead(BUTTON_PIN) == LOW) {
    pressStart = millis();
    buttonPressed = true;
  } else {
    if (buttonPressed) {
      pressDuration = millis() - pressStart;
      buttonPressed = false;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);          
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  pinMode(LED_BMP, OUTPUT);
  pinMode(LED_BTN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButton, CHANGE);

  if (!bmp.begin(0x76)) {
    Serial.println("Не вдалось знайти BMP280!");
    while (1);
  }

  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,   
    Adafruit_BMP280::SAMPLING_X16,  
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );
  Serial.println("Система готова. BMP280 та кнопка працюють.");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastBMPRead >= bmpInterval) {
    lastBMPRead = currentMillis;
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0;
    Serial.print("Температура: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Тиск: ");
    Serial.print(pressure);
    Serial.println(" hPa");
    
    digitalWrite(LED_BMP, HIGH);
    delay(100);
    digitalWrite(LED_BMP, LOW);
  }

  if (pressDuration > 0) {
    unsigned long duration = pressDuration;
    pressDuration = 0; 
    Serial.print("Тривалість натискання кнопки: ");
    Serial.print(duration);
    Serial.println(" мс");
    digitalWrite(LED_BTN, HIGH);
    delay(duration);
    digitalWrite(LED_BTN, LOW);
  }
}






