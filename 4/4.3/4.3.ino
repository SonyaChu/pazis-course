#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4          
#define NUM_SAMPLES 100         
#define READ_INTERVAL_MS 750    
#define MAX_NOISE 1.0           

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatureSamples[NUM_SAMPLES]; 
float filteredSamples[NUM_SAMPLES];    
DeviceAddress tempDeviceAddress; 
int numberOfDevices = 0;

void printAddress(DeviceAddress deviceAddress);
void applyKalmanFilter(float Q, float R);
 
void setup() {
  Serial.begin(115200);
  Serial.println("Завдання 3. Фільтр Кальмана з симуляцією шуму");
  Serial.println("---------------------------------------------------------");
  randomSeed(analogRead(0)); 
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  
  if (numberOfDevices == 0 || !sensors.getAddress(tempDeviceAddress, 0)) {
    Serial.println("Помилка: Не знайдено DS18B20. Перевірте підключення.");
    while(true); 
  }
  
  Serial.print("Використовується пристрій 0. Затримка зчитування: ");
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
      Serial.print(", З шумом ");
      Serial.print(noisyTempC, 2);
      Serial.println(" C");
    }
  }

  Serial.println("\n---------------------------------------------------------------");
  Serial.println("2. Застосування фільтра Кальмана:");
  float Q_COV = 0.001; 
  float R_COV = 0.5;   
  applyKalmanFilter(Q_COV, R_COV);
  Serial.println("\n---------------------------------------------------------------");
  Serial.println("3. Порівняння ЗШУМЛЕНИХ та ВІДФІЛЬТРОВАНИХ даних:");
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
 =========================================================================================
void loop() {
  delay(10000); 
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

void applyKalmanFilter(float Q, float R) {
  float x_est = temperatureSamples[0]; 
  float P = 0.1;                      
  Serial.print("Ініціалізація: Q=");
  Serial.print(Q, 4);
  Serial.print(", R=");
  Serial.print(R, 4);
  Serial.print(", Початкова оцінка=");
  Serial.print(x_est, 2);
  Serial.println(" C.");

  for (int i = 0; i < NUM_SAMPLES; i++) {
    float measurement = temperatureSamples[i];
    P = P + Q; 
    float K = P / (P + R);
    x_est = x_est + K * (measurement - x_est);
    P = (1 - K) * P;
    filteredSamples[i] = x_est;
  }
  Serial.println("Фільтрацію завершено.");
}