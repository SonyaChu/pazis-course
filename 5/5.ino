#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "FS.h"
#include "SPIFFS.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h> 
#include <BLE2901.h> 

#define DATA_FILE "/sensor_data.csv"
const long MEASURE_INTERVAL_MS = 15000; 
unsigned long previousMillis = 0;

int LDR_Pin = 13;
#define ONE_WIRE_BUS 4

#define SERVICE_UUID        "59d0c201-1fb5-459e-8fcc-c4c46c47894a"
#define DS18B20_TEMP_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a1"
#define BMP280_TEMP_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a2"
#define BMP280_PRESS_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a3"
#define LDR_VAL_UUID        "beb5483e-36e1-4688-b7f5-ea07361b26a4"

Adafruit_BMP280 bmp;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
BLECharacteristic *pDS18B20Char;
BLECharacteristic *pBMP280TempChar;
BLECharacteristic *pBMP280PressChar;
BLECharacteristic *pLDRChar;

void initSPIFFS();
void appendDataToFile(const char * path, const char * message);
String formatDataCSV(float dsTemp, float bmpTemp, float pressure, int ldrValue);
String readLastLine(const char * path);
void updateBLECharacteristics(String lastData);

void setup() {
  delay(1000); 
  Serial.begin(115200);
  Serial.println("--- СТАРТ ПРОГРАМИ: Завдання 1 (BLE + SPIFFS) ---");

  if (!bmp.begin(0x76)) {
    Serial.println("Помилка: Не вдалось знайти BMP280! Перевірте підключення.");
  }
  sensors.begin();
  pinMode(LDR_Pin, INPUT);
  
  initSPIFFS();
  if (!SPIFFS.exists(DATA_FILE)) {
    String header = "Timestamp_ms,DS18B20_T(C),BMP280_T(C),BMP280_P(hPa),LDR_ADC\n";
    appendDataToFile(DATA_FILE, header.c_str());
  }

  Serial.println("Налаштування BLE-сервера...");
  BLEDevice::init("Метеостанція_ESP32"); 
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pDS18B20Char = pService->createCharacteristic(DS18B20_TEMP_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pBMP280TempChar = pService->createCharacteristic(BMP280_TEMP_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pBMP280PressChar = pService->createCharacteristic(BMP280_PRESS_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pLDRChar = pService->createCharacteristic(LDR_VAL_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  BLEDescriptor *p2901Ds = new BLE2901();
  p2901Ds->setValue("Температура (DS18B20)");
  pDS18B20Char->addDescriptor(p2901Ds);
  pDS18B20Char->addDescriptor(new BLE2902());
  
  BLEDescriptor *p2901BmpT = new BLE2901();
  p2901BmpT->setValue("Температура (BMP280)");
  pBMP280TempChar->addDescriptor(p2901BmpT);
  pBMP280TempChar->addDescriptor(new BLE2902());

  BLEDescriptor *p2901BmpP = new BLE2901();
  p2901BmpP->setValue("Тиск (BMP280)");
  pBMP280PressChar->addDescriptor(p2901BmpP);
  pBMP280PressChar->addDescriptor(new BLE2902());

  BLEDescriptor *p2901Ldr = new BLE2901();
  p2901Ldr->setValue("Освітленість (LDR)"); 
  pLDRChar->addDescriptor(p2901Ldr);
  pLDRChar->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("BLE-сервер запущено. Очікування клієнта...");
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
    Serial.println("\n--- Новий збір даних ---");
    Serial.printf("DS18B20 T: %.2f °C, BMP280 T: %.2f °C, P: %.2f hPa, LDR: %d\n", dsTemp, bmpTemp, pressure, ldrValue);
    String dataLine = formatDataCSV(dsTemp, bmpTemp, pressure, ldrValue);
    appendDataToFile(DATA_FILE, dataLine.c_str());
    String lastData = readLastLine(DATA_FILE);
    updateBLECharacteristics(lastData);
  }
  delay(100); 
}

void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Помилка: Помимилка ініціалізації SPIFFS");
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

String readLastLine(const char * path) {
  File file = SPIFFS.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
    return ""; 
  }
  
  long fileSize = file.size();
  if (fileSize < 20) { 
    file.close();
    return "";
  }

  long pos = fileSize - 1; 
  int newlineCount = 0;
  
  while (pos >= 0) {
    file.seek(pos);
    char c = file.read();

    if (c == '\n') {
      newlineCount++;
      if (newlineCount == 2) { 
        break;
      }
    }
    pos--;
  }

  file.seek(pos + 1);
  String lastLine = file.readStringUntil('\n');
  file.close();
  lastLine.trim(); 
  return lastLine;
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

void updateBLECharacteristics(String lastData) {
  if (lastData.isEmpty()) { 
    Serial.println("Помилка: Не вдалось прочитати дані для BLE (файл порожній або некоректний).");
    return;
  }
  Serial.print("Останні дані для BLE: ");
  Serial.println(lastData);

  int commaIndex1 = lastData.indexOf(',');
  int commaIndex2 = lastData.indexOf(',', commaIndex1 + 1);
  int commaIndex3 = lastData.indexOf(',', commaIndex2 + 1);
  int commaIndex4 = lastData.indexOf(',', commaIndex3 + 1);
  
  if (commaIndex4 == -1) {
      Serial.println("Помилка: Некоректний формат CSV для парсингу. Рядок неповний.");
      return;
  }

  String dsTempStr = lastData.substring(commaIndex1 + 1, commaIndex2);
  String bmpTempStr = lastData.substring(commaIndex2 + 1, commaIndex3);
  String pressureStr = lastData.substring(commaIndex3 + 1, commaIndex4);
  String ldrValStr = lastData.substring(commaIndex4 + 1);
  
  pDS18B20Char->setValue(dsTempStr.c_str());
  pBMP280TempChar->setValue(bmpTempStr.c_str());
  pBMP280PressChar->setValue(pressureStr.c_str());
  pLDRChar->setValue(ldrValStr.c_str());

  pDS18B20Char->notify();
  pBMP280TempChar->notify();
  pBMP280PressChar->notify();
  pLDRChar->notify();
  
  Serial.println("BLE Характеристики оновлено.");
}
