#include <Wire.h>
#include <Adafruit_BMP280.h>  


Adafruit_BMP280 bmp;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  if (!bmp.begin(0x76)) { 
    Serial.println("Не вдалось знайти BMP280!");
    while (1);
  }

  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,   
                  Adafruit_BMP280::SAMPLING_X16, 
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void loop() {
  float temperature = bmp.readTemperature();   
  float pressure = bmp.readPressure() / 100.0; 
  Serial.print("Температура: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Тиск: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  delay(2000);

}
