#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfDevices;
DeviceAddress tempDeviceAddress;

void setup() {
  Serial.begin(115200);
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  Serial.print("Пошук пристроїв...");
  Serial.print("Знайдено ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" пристроїв.");

  for(int i=0; i<numberOfDevices; i++){
    if(sensors.getAddress(tempDeviceAddress, i)){
      Serial.print("Знайдено пристрій ");
      Serial.print(i, DEC);
      Serial.print(" з адресою: ");
      printAddress(tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Знайдено «привид» на позиції ");
      Serial.print(i, DEC);
      Serial.print(", але не вдалося визначити адресу. Перевір живлення та дроти");
    }
  }
}

void loop() {
  sensors.requestTemperatures(); 
  for(int i=0; i<numberOfDevices; i++){
    if(sensors.getAddress(tempDeviceAddress, i)){
      Serial.print("Температура пристрою: ");
      Serial.println(i, DEC);
      float tempC = sensors.getTempC(tempDeviceAddress);
      Serial.print("Темп C: ");
      Serial.print(tempC);
      Serial.print(" Темп F: ");
      Serial.println(DallasTemperature::toFahrenheit(tempC)); 
    }
  }
  delay(5000);
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}