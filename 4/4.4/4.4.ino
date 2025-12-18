#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "FS.h"             
#include "SPIFFS.h"         

#define DATA_FILE "/sensor_data.csv"
const long MEASURE_INTERVAL_MS = 15000; 
unsigned long previousMillis = 0;
static int read_file_counter = 0; 

int LDR_Pin = 13;           
#define ONE_WIRE_BUS 4      

Adafruit_BMP280 bmp;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void initSPIFFS();
void appendDataToFile(const char * path, const char * message);
void readFile(const char * path);
String formatDataCSV(float dsTemp, float bmpTemp, float pressure, int ldrValue);

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!bmp.begin(0x76)) { 
    Serial.println("Помилка: Не вдалось знайти BMP280!");
    while (1);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                   Adafruit_BMP280::SAMPLING_X2,
                   Adafruit_BMP280::SAMPLING_X16,
                   Adafruit_BMP280::FILTER_X16,
                   Adafruit_BMP280::STANDBY_MS_500);

  sensors.begin();
  pinMode(LDR_Pin, INPUT);
  Serial.println("Система готова до роботи!");
  initSPIFFS();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MEASURE_INTERVAL_MS) {
    previousMillis = currentMillis;
    sensors.requestTemperatures();
    float dsTemp = sensors.getTempCByIndex(0);
    float bmpTemp = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0;
    int ldrValue = analogRead(LDR_Pin);
    Serial.println("\n---------------------------------------");
    Serial.println("Збір даних:");
    Serial.printf("DS18B20 T: %.2f °C\n", dsTemp);
    Serial.printf("BMP280 T: %.2f °C, P: %.2f hPa\n", bmpTemp, pressure);
    Serial.printf("LDR (ADC): %d\n", ldrValue);
    Serial.println("---------------------------------------");
    String dataLine = formatDataCSV(dsTemp, bmpTemp, pressure, ldrValue);
    appendDataToFile(DATA_FILE, dataLine.c_str());
    read_file_counter++;
    if (read_file_counter >= 3) {
      Serial.println("\n***************************************************");
      Serial.println("Програмне відкриття та читання файлу (для скріншоту)");
      readFile(DATA_FILE);
      Serial.println("***************************************************");
      read_file_counter = 0; 
    }
  }
}

void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Помилка: Помилка ініціалізації SPIFFS");
    return;
  }
  Serial.println("SPIFFS ініціалізовано успішно.");
}

void appendDataToFile(const char * path, const char * message) {
  File file = SPIFFS.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Помилка: Помилка відкриття файлу для запису");
    return;
  }
  if (file.print(message)) {
    Serial.println("Дані записано у файл.");
  } else {
    Serial.println("Помилка запису даних.");
  }
  file.close();
}

void readFile(const char * path) {
  File file = SPIFFS.open(path, FILE_READ);
  if (!file) {
    Serial.println("Помилка: Помилка відкриття файлу для читання");
    return;
  }
  Serial.println("--- Вміст файлу sensor_data.csv ---");
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println("-------------------------------------");
  file.close();
}

String formatDataCSV(float dsTemp, float bmpTemp, float pressure, int ldrValue) {
  String dataLine = "";
  dataLine += String(millis());
  dataLine += ",";
  dataLine += String(dsTemp, 2);
  dataLine += ",";
  dataLine += String(bmpTemp, 2);
  dataLine += ",";
  dataLine += String(pressure, 2);
  dataLine += ",";
  dataLine += String(ldrValue);
  dataLine += "\n"; 
  return dataLine;
}