#define LDR_PIN 34          
#define LED_PIN 32        
#define BUTTON1_PIN 33      
#define BUTTON2_PIN 25      

volatile bool buttonReleased = false; 
volatile bool button2Pressed = false;
volatile unsigned long lastInterruptTime1 = 0; 
volatile unsigned long lastInterruptTime2 = 0; 
#define DEBOUNCE_TIME 200  
volatile int buttonCount = 0; 

void IRAM_ATTR handleButtonRelease1() {
  unsigned long now = millis();
  if (now - lastInterruptTime1 > DEBOUNCE_TIME) {
    buttonReleased = true;
    lastInterruptTime1 = now;
  }
}

void IRAM_ATTR handleButtonPress2() {
  unsigned long now = millis();
  if (now - lastInterruptTime2 > DEBOUNCE_TIME) {
    button2Pressed = true;
    lastInterruptTime2 = now;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(BUTTON1_PIN, handleButtonRelease1, RISING);  
  attachInterrupt(BUTTON2_PIN, handleButtonPress2, FALLING);   
  Serial.println("Система готова. Натискай кнопки!");
}

void loop() {
  
  if (buttonReleased) {
    buttonReleased = false;
    int ldrValue = analogRead(LDR_PIN);
    Serial.print("Освітленість (LDR): ");
    Serial.println(ldrValue);
    analogWrite(LED_PIN, 255);
    delay(300);          
    analogWrite(LED_PIN, 0);
    delay(200);          
    Serial.println("Переривання №1 виконано.");
    Serial.println("---------------------------------------");
  }

  if (button2Pressed) {
    button2Pressed = false;
    buttonCount++;
    Serial.print("Кнопку №2 натиснуто разів: ");
    Serial.println(buttonCount);
    if (buttonCount % 2 == 0) {
      analogWrite(LED_PIN, 0); 
      Serial.println("LED вимкнено");
    } else {
      int brightness = (buttonCount % 10) * 25; 
      analogWrite(LED_PIN, brightness);
      Serial.print("LED увімкнено, яскравість = ");
      Serial.println(brightness);
    }
    Serial.println("---------------------------------------");
  }
}
