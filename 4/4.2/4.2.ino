#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4          
#define NUM_SAMPLES 100         
#define READ_INTERVAL_MS 750    
#define WINDOW_SIZE 3           
#define MAX_NOISE 1.0           

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatureSamples[NUM_SAMPLES]; 
DeviceAddress tempDeviceAddress; 
int numberOfDevices = 0;

void printAddress(DeviceAddress deviceAddress);
void medianFilter(float *inputArray, float *outputArray, int dataSize, int windowSize);
void simpleSort(float *arr, int size);

void setup() {
  Serial.begin(115200);
  Serial.println("Завдання 2. Медіанний фільтр з симуляцією імпульсного шуму");
  Serial.println("---------------------------------------------------------");
  randomSeed(analogRead(0)); 
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  
  if (numberOfDevices == 0 || !sensors.getAddress(tempDeviceAddress, 0)) {
    Serial.println("Помилка: Не знайдено DS18B20. Перевірте підключення.");
    while(true); 
  }
  
  Serial.print("Використовується пристрій 0. Мінімальна затримка: ");
  Serial.print(READ_INTERVAL_MS);
  Serial.println(" мс.");
  Serial.print("Імітація шуму: +/- ");
  Serial.print(MAX_NOISE, 1);
  Serial.println(" C.");
  Serial.print("\n1. Зчитування ");
  Serial.print(NUM_SAMPLES);
  Serial.println(" значень...");

  for (int i = 0; i < NUM_SAMPLES; i++) {
    sensors.requestTemperatures(); 
    delay(READ_INTERVAL_MS); 
    float actualTempC = sensors.getTempC(tempDeviceAddress);
    long randomRange = 2 * (long)(MAX_NOISE * 100); 
    float noise = ( (float)random(randomRange) / 100.0 ) - MAX_NOISE; 
    float noisyTempC = actualTempC + noise;
    
    if (actualTempC == DEVICE_DISCONNECTED_C) {
      Serial.print("Помилка зчитування на позиції ");
      Serial.println(i);
      temperatureSamples[i] = 0.0;
    } else {
      temperatureSamples[i] = noisyTempC; 
      Serial.print("Зчитано ");
      Serial.print(i + 1);
      Serial.print(": Реальна ");
      Serial.print(actualTempC, 2);
      Serial.print(", Шум ");
      Serial.print(noise, 2);
      Serial.print(", З шумом ");
      Serial.print(noisyTempC, 2);
      Serial.println(" C");
    }
  }

  Serial.println("\n---------------------------------------------------------------");
  Serial.println("2. Застосування медіанного фільтру (Розмір вікна: 3)...");
  float filteredSamples[NUM_SAMPLES];
  medianFilter(temperatureSamples, filteredSamples, NUM_SAMPLES, WINDOW_SIZE);
  Serial.println("\n---------------------------------------------------------------");
  Serial.println("3. Порівняння ЗШУМЛЕНИХ та відфільтрованих даних:");
  Serial.println("Індекс | ЗШУМЛЕНЕ (C) | Фільтр (C) | Різниця");
  Serial.println("---------------------------------------------------------------");

  for (int i = 0; i < NUM_SAMPLES; i++) {
    Serial.print(i, DEC);
    Serial.print("     | ");
    Serial.print(temperatureSamples[i], 2);
    Serial.print("         | ");
    Serial.print(filteredSamples[i], 2);
    Serial.print("         | ");
    Serial.println(temperatureSamples[i] - filteredSamples[i], 2); 
  }
}

void loop() {
  
  delay(10000); 
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

void simpleSort(float *arr, int size) {
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        float temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

void medianFilter(float *inputArray, float *outputArray, int dataSize, int windowSize) {
  int offset = windowSize / 2;

  if (windowSize % 2 == 0) {
    Serial.println("ПОМИЛКА: Розмір вікна медіанного фільтра має бути НЕПАРНИМ!");
    return;
  }
  
  for (int i = 0; i < dataSize; i++) {
    int start = i - offset;
    int end = i + offset;
    float windowBuffer[windowSize];
    int windowIndex = 0;

    for (int j = start; j <= end; j++) {
      float value;
      if (j < 0) {
        value = inputArray[0];
      } else if (j >= dataSize) {
        value = inputArray[dataSize - 1];
      } else {
        value = inputArray[j];
      }
    
      windowBuffer[windowIndex++] = value;
    }

    simpleSort(windowBuffer, windowSize);
    outputArray[i] = windowBuffer[offset];
  }
}