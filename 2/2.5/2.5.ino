#include <Wire.h>
#include <Adafruit_BMP280.h>      
#include <OneWire.h>
#include <DallasTemperature.h>    

int LDR_Pin = 13;    
int LED_Pin = 25;    
int LDR_Value = 0;

Adafruit_BMP280 bmp;

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!bmp.begin(0x76)) {
    Serial.println("Не вдалось знайти BMP280!");
    while (1);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,   
                  Adafruit_BMP280::SAMPLING_X16,  
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

  sensors.begin();
  pinMode(LDR_Pin, INPUT);
  pinMode(LED_Pin, OUTPUT);
  Serial.println("Система готова до роботи!");
}

void loop() {
  Serial.println("---------------------------------------");
  LDR_Value = analogRead(LDR_Pin);
  Serial.print("Освітленість (ADC): ");
  Serial.println(LDR_Value);
  int dacValue = map(LDR_Value, 0, 4095, 0, 255);
  dacWrite(LED_Pin, dacValue);
  Serial.println("---------------------------------------");
  float bmpTemp = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0;
  Serial.print("Температура BMP280: ");
  Serial.print(bmpTemp);
  Serial.println(" °C");
  Serial.print("Тиск BMP280: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  Serial.println("---------------------------------------");
  sensors.requestTemperatures();
  float dsTemp = sensors.getTempCByIndex(0);
  Serial.print("Температура DS18B20: ");
  Serial.print(dsTemp);
  Serial.println(" °C");
  Serial.println("---------------------------------------");
  delay(3000);// 
}
