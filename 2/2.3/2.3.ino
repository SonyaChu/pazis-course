#include <Adafruit_BMP280.h>
#include <SPI.h>

#define BMP_CS   5    
#define BMP_MOSI 23   
#define BMP_MISO 19   
#define BMP_SCK  18   

Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO, BMP_SCK); 

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!bmp.begin()) {
    Serial.println("Не знайдено BMP280! Перевірте підключення.");
    while (1);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X4,   
                  Adafruit_BMP280::SAMPLING_X4,   
                  Adafruit_BMP280::FILTER_X4,     
                  Adafruit_BMP280::STANDBY_MS_500 
                 );

  Serial.println("BMP280 готовий до роботи.");
}

void loop() {
  float temperature = bmp.readTemperature();         
  float pressure_hPa = bmp.readPressure() / 100.0F;  

  Serial.print("Температура: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Тиск: ");
  Serial.print(pressure_hPa);
  Serial.println(" hPa");

  
  delay(2000);
}
